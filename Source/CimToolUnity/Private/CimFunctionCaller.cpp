// Fill out your copyright notice in the Description page of Project Settings.


#include "CimFunctionCaller.h"

#include "JsonLibraryObject.h"


template<typename... TReturns, typename... TArgs>
void UCimFunctionCaller::Caller_Internal(UClass* OuterClass, UFunction* Function, TTuple<TReturns...>& OutParams, TArgs&&... Args)
{
  uint8* OutParamsBytes = (uint8*)&OutParams; // the addr of the in Tuple OutParams, which is the return value of the function
  TTuple<TArgs...> InParams(Forward<TArgs>(Args)...);
  uint8* InParamsBytes = (uint8*)&InParams; // get the addr of the in Tuple InParams

  void* FuncParamsStructAddr = (uint8*)FMemory_Alloca(Function->ParmsSize);   // prepare allocated ufunction parameters memory  

  const bool bHasReturnParam = Function->ReturnValueOffset != MAX_uint16; //MAX_uint16, which is a special value used to indicate that there is no return value.
  const int32 SizeIn = bHasReturnParam ? Function->ReturnValueOffset : Function->ParmsSize;
  FMemory::Memcpy(FuncParamsStructAddr, InParamsBytes, SizeIn); // copy the InParams to the FuncParams address, ensure the size.

  if (Function->HasAllFunctionFlags(FUNC_Native)) // The HasAllFunctionFlags(FUNC_Native) check in your code is used to determine if the function being processed is a native function. If it is, the function is invoked directly. If it's not, the function is processed through Unreal's event system with OuterClass->ProcessEvent(Function, FuncParamsStructAddr).
  {
    FFrame Frame(nullptr, Function, FuncParamsStructAddr, nullptr, Function->ChildProperties); // UE reflection system structure, found in Stack.h

    FOutParmRec** LastOut = &Frame.OutParms; //LastOut initially points to the OutParms field of the FFrame object, which is the head of the linked list of FOutParmRec objects.
    for (TFieldIterator<FProperty> ParamIt(Function); ParamIt; ++ParamIt) // get all reflection properties of the UFunction using this iterator, see dazhao's answer
    {
      FProperty* Property = *ParamIt;
      if ((Property->PropertyFlags & CPF_OutParm) == CPF_OutParm && (Property->PropertyFlags & CPF_ReturnParm) == 0) 
      {
        FOutParmRec* Out = (FOutParmRec*)FMemory_Alloca(sizeof(FOutParmRec)); //在这里为每个需要的属性分配了FOutParmRec，从而去构建Frame.OutParam的属性链表。 set the property ptr offset and the value
        void* PropAddr = Property->ContainerPtrToValuePtr<void*>(FuncParamsStructAddr);
        FMemory::Memcpy(PropAddr, InParamsBytes + Property->GetOffset_ForInternal(), Property->GetSize());
        Out->PropAddr = (uint8*)PropAddr;
        Out->Property = Property;
        if (*LastOut) // this is the pointer to "&Frame.OutParms" which is already have things.
        {
          (*LastOut)->NextOutParm = Out;
          LastOut = &(*LastOut)->NextOutParm; // this is to update the pointer position to new Setted OutParm.
        }
        else // which means the OutParms is empty
        {
          *LastOut = Out;
        }
      } // so , we update a new FOutParmRec object (LastOut) , which will construct the right FFrame object
    }

    //如果有返回值，确定返回值的地址
    uint8* ReturnValueAddress = bHasReturnParam ? (uint8*)FuncParamsStructAddr + Function->ReturnValueOffset : nullptr;
    Function->Invoke(OuterClass, Frame, ReturnValueAddress); // it does execute if the addr is nullptr

    //复制返回参数，包括引用类型的参数, // and set the return value, copy every OutParams to the OutParamsBytes
    OutParamsBytes = (uint8*)&OutParams;
    for (TFieldIterator<FProperty> It(Function); It; ++It)
    {
      FProperty* prop = *It;
      if (prop->PropertyFlags & CPF_OutParm)
      {
        const void* propBuffer = prop->ContainerPtrToValuePtr<void*>(FuncParamsStructAddr);
        prop->CopyCompleteValue(OutParamsBytes, propBuffer);
        OutParamsBytes += prop->GetSize();
      }
    }
    return;
  }

  OuterClass->ProcessEvent(Function, FuncParamsStructAddr); // blueprint VM will do the FFrame process
  
  OutParamsBytes = (uint8*)&OutParams;
  for (TFieldIterator<FProperty> It(Function); It; ++It)
  {
    FProperty* Property = *It;
    if (Property->PropertyFlags & CPF_OutParm)
    {
      void* PropAddr = Property->ContainerPtrToValuePtr<void*>(FuncParamsStructAddr);
      FMemory::Memcpy(OutParamsBytes, PropAddr, Property->GetSize());
      OutParamsBytes += Property->GetSize();
    }
  }

  // FMemory::Free(FuncParamsStructAddr); // you can't do that!
  
}


template<typename... TReturns, typename... TArgs>
void UCimFunctionCaller::CallInternal3(UClass* OuterClass, UFunction* Function, TTuple<TReturns...>& OutParams, TArgs&&... Args)
{
  uint8* OutParamsBytes = (uint8*)&OutParams;
  TTuple<TArgs...> InParams(Forward<TArgs>(Args)...);
  uint8* InParamsByte = (uint8*)&InParams;

  void* FuncParamsStructAddr = (uint8*)FMemory_Alloca(Function->ParmsSize);
  FMemory::Memzero(FuncParamsStructAddr, Function->ParmsSize);
  for (TFieldIterator<FProperty> It(Function); It; ++It)
  {
    FProperty* Property = *It;
    void* PropAddr = Property->ContainerPtrToValuePtr<void*>(FuncParamsStructAddr);
    if ((Property->PropertyFlags & CPF_Parm) == CPF_Parm && (Property->PropertyFlags & CPF_ReturnParm) == 0)
    {
      FMemory::Memcpy(PropAddr, InParamsByte + Property->GetOffset_ForInternal(), Property->GetSize());
    }
  }

  OuterClass->ProcessEvent(Function, FuncParamsStructAddr);
  OutParamsBytes = (uint8*)&OutParams;
  for (TFieldIterator<FProperty> i(Function); i; ++i)
  {
    FProperty* prop = *i;
    if (prop->PropertyFlags & CPF_OutParm)
    {
      const void* propBuffer = prop->ContainerPtrToValuePtr<void*>(FuncParamsStructAddr);
      prop->CopyCompleteValue(OutParamsBytes, propBuffer);
      OutParamsBytes += prop->GetSize();
    }
  }
}

void UCimFunctionCaller::Caller(const FString& Name, const FString& InDescriptor,  FString& Returns)
{
  // find the function or event or interface by name
  const FJsonLibraryObject Descriptor = FJsonLibraryObject::Parse(InDescriptor);
	
  const FJsonLibraryValue Function = Descriptor.GetValue("command");

	
  const FJsonLibraryValue Options_Object = Descriptor.GetValue("options");
	
  const FJsonLibraryValue Args = Options_Object.GetObject().GetValue("args");
  const FJsonLibraryValue MessageID = Options_Object.GetObject().GetValue("messageId");
  // FJsonLibraryObject x =  Descriptor
  GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Function: %s"), *Function.GetString()));
  // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Args: %s"), *Args.Stringify()));

  FJsonLibraryObject ArgsObject = Args.GetObject();
  
  // pass the args

  return;
}
