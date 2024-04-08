
#pragma once
#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"



DEFINE_LOG_CATEGORY_STATIC(DebugHeader, Log, All);


#define DEBUG_TEXT(TextToShow) FString::Printf(TEXT("[DebugHeader] : %s"), *TextToShow)
// get the utc time


enum class EBugSeverity : uint8
{
	EInfo,
	EWarning,
	EError
};




inline void CIMTOOLUNITY_API DebugThis(const FString& TextToShow, const EBugSeverity Severity)
{
	check(!TextToShow.IsEmpty())
	if(GEngine)
	{
		switch (Severity)
		{
		case EBugSeverity::EInfo:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DEBUG_TEXT(TextToShow));
			break;
		case EBugSeverity::EWarning:
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, DEBUG_TEXT(TextToShow));
			UE_LOG(DebugHeader, Warning, TEXT("%s"), *TextToShow);
			break;
		case EBugSeverity::EError:
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, DEBUG_TEXT(TextToShow));
			UE_LOG(DebugHeader, Error, TEXT("%s"), *TextToShow);
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TextToShow);
			break;
		}

	}
	return;
}



inline void CIMTOOLUNITY_API DebugPrint(const FString& TextToShow)
{
	check(!TextToShow.IsEmpty())
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DEBUG_TEXT(TextToShow));
}


inline EAppReturnType::Type CIMTOOLUNITY_API ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, 
bool bShowMsgAsWarning = true)
{	
	if(bShowMsgAsWarning)
	{
		const FText MsgTitle = FText::FromString(TEXT("Warning"));

		return FMessageDialog::Open(MsgType,FText::FromString(Message),&MsgTitle);
	}
	else
	{
		return FMessageDialog::Open(MsgType,FText::FromString(Message));
	}			
}

inline void CIMTOOLUNITY_API ShowNotifyInfo(const FString& Message)
{
	FNotificationInfo NotifyInfo(FText::FromString(Message));
	NotifyInfo.bUseLargeFont = true;
	NotifyInfo.FadeOutDuration = 7.f;

	FSlateNotificationManager::Get().AddNotification(NotifyInfo);
}

