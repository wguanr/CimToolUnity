// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActor.h"
#include "CimFunctionCaller.h"
#include "DebugHeader.h"

// Sets default values
ATestActor::ATestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATestActor::TestFunc()
{
	// jsut print the log
	UE_LOG(LogTemp, Warning, TEXT("TestFunc"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("TestFunc"));
}

void ATestActor::TestFunc2(const FString& str)
{
	//print the str
	UE_LOG(LogTemp, Warning, TEXT("TestFunc2: %s"), *str);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("TestFunc2: %s"), *str));
}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	TTuple<void*> OutParams;
	// UCimFunctionCaller::Caller_Internal(this->GetClass(), this->FindFunction(FName("TestFunc")), OutParams);
	// UCimFunctionCaller::CallInternal3(this->GetClass(), this->FindFunction(FName("TestFunc2")), OutParams, FString("Hello World"));
	// find uclass object and call the function
	if (UClass* TestClass =
		FindObject<UClass>(ANY_PACKAGE, TEXT("NewBlueprint_C")))
	{
			// UCimFunctionCaller::Caller_Internal(TestClass, TestClass->FindFunctionByName(FName("T4")), OutParams, true);

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find the class"));
	
	}
	TEXT("/Script/Engine.Blueprint'/Game/Developers/ue/NewFunctionLibrary.NewFunctionLibrary'");
	TArray<UObject*> Objects;
	GetObjectsOfClass(UBlueprintFunctionLibrary::StaticClass(), Objects);
	UClass* TestClass = LoadClass<UBlueprintFunctionLibrary>(nullptr, TEXT("/Game/Developers/ue/NewFunctionLibrary.NewFunctionLibrary_C"));
	if (TestClass)
	{
		DebugThis(TestClass->GetClass()->GetName(),EBugSeverity::EWarning);
		UCimFunctionCaller::Caller_Internal(TestClass->GetClass(), TestClass->FindFunctionByName(FName("Test")), OutParams, true);
		
	}
	
	// get the utc time
	FDateTime CurrentTime = FDateTime::UtcNow();
	UE_LOG(LogTemp, Warning, TEXT("UTC time: %s"), *CurrentTime.ToString());
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

