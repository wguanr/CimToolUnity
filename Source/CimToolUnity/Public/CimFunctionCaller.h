// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CimFunctionCaller.generated.h"

/**
 * 
 */
UCLASS()
class CIMTOOLUNITY_API UCimFunctionCaller : public UObject
{
	GENERATED_BODY()

public:
	template<typename... TReturns, typename... TArgs>
	static void Caller_Internal(UClass* OuterClass, UFunction* Function, TTuple<TReturns...>& OutParams, TArgs&&... Args);

	template<typename... TReturns, typename... TArgs>
	static void CallInternal3(UClass* OuterClass, UFunction* Function, TTuple<TReturns...>& OutParams, TArgs&&... Args);

	UFUNCTION()
	static void Caller( UPARAM(ref) const FString& Name, UPARAM(ref) const FString& InDescriptor, FString& JsonStyleReturns);
};


