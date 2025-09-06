// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserInterfaceBlueprintLibrary.h"

#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonUITypes.h"
#include "GameplayTagContainer.h"
#include "UI/RogueGameLayout.h"
#include "UI/RogueHUD.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/NavigationConfig.h"

#include "Misc/ConfigCacheIni.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(UserInterfaceBlueprintLibrary)

DEFINE_LOG_CATEGORY(LogUserInterfaceBlueprintLibrary);

UCommonActivatableWidget* UUserInterfaceBlueprintLibrary::PushContentToLayerForPlayer(const APlayerController* Player, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!Player || WidgetClass == nullptr)
	{
		UE_LOG(LogUserInterfaceBlueprintLibrary, Error, TEXT("UUserInterfaceBlueprintLibrary::PushContentToLayerForPlayer, invalid player parameter or widget class returning nullptr."));
		return nullptr;
	}

	// Get the player's HUD and attempt to create and return the widget
	if (ARogueHUD* RogueHUD = Player->GetHUD<ARogueHUD>())
	{
		if (URogueGameLayout* RootLayout = RogueHUD->GetRootLayoutWidget())
		{
			UCommonActivatableWidget* NewWidget = RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);
			return NewWidget;
		}
	}

	return nullptr;
}

void UUserInterfaceBlueprintLibrary::SetNavigationEnabled(bool bIsNavEnabled)
{
	FSlateApplication::Get().GetNavigationConfig()->bTabNavigation = bIsNavEnabled;
	FSlateApplication::Get().GetNavigationConfig()->bKeyNavigation = bIsNavEnabled;
	FSlateApplication::Get().GetNavigationConfig()->bAnalogNavigation = bIsNavEnabled;
}

FString UUserInterfaceBlueprintLibrary::GetProjectVersion()
{
	FString AppVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		AppVersion,
		GGameIni
	);

	return AppVersion; 
}
