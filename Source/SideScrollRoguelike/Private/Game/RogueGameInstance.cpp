// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/RogueGameInstance.h"

#include "Framework/Application/SlateApplication.h"
#include "GameMapsSettings.h"
#include "LoadingScreenManager.h"

#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameInstance)

namespace
{
    static const FName& GetDefaultMapName()
    {
        static FName NAME_DefaultMap = *([]()
                                         { return FPaths::GetCleanFilename(UGameMapsSettings::GetGameDefaultMap()); })();
        return NAME_DefaultMap;
    }
} // namespace

bool URogueGameInstance::HasPlayedBootSplash() const
{
    return bHasPlayedBootSplash;
}

void URogueGameInstance::SetHasPlayedBootSplash(bool bPlayed)
{
    bHasPlayedBootSplash = bPlayed;
}

void URogueGameInstance::HoldLoadingScreen(bool bHold)
{
    bHoldLoadingScreen = bHold;
}

bool URogueGameInstance::ShouldHoldLoadingScreen() const
{
    return bHoldLoadingScreen;
}

void URogueGameInstance::Init()
{
    Super::Init(); // will init all the subsystems as well

    // Bind to slate delegate to be notified of window focus changes
    // Credit to BenUI https://benui.ca/unreal/window-focus-change/
    FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &ThisClass::WindowFocusChanged);

    if (ULoadingScreenManager* LoadingScreenManager = GetSubsystem<ULoadingScreenManager>())
    {
        // Loading processors are stored as weak pointers, so we don't have to worry about cleaning this up
        LoadingScreenManager->RegisterLoadingProcessor(this);
    }
}

#if WITH_EDITOR
FGameInstancePIEResult URogueGameInstance::InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params)
{
    FGameInstancePIEResult Result = Super::InitializeForPlayInEditor(PIEInstanceIndex, Params);

    // Don't bother with the boot splash in PIE unless we are intentionally loading into the main menu
    if (GetWorld()->GetName() != GetDefaultMapName())
    {
        bHasPlayedBootSplash = true;
    }

    return Result;
}
#endif

bool URogueGameInstance::ShouldShowLoadingScreen(FString& OutReason) const
{
    if (ShouldHoldLoadingScreen())
    {
        OutReason = TEXT("RogueGameInstance is holding the loading screen");
        return true;
    }
    return false;
}

void URogueGameInstance::WindowFocusChanged(bool bIsFocused)
{
    // Don't pause in the editor, it's annoying
#if !WITH_EDITOR
    if (bIsFocused)
    {
        // Unlimit game FPS
        GEngine->SetMaxFPS(0);
    }
    else
    {
        // Reduce FPS to max 10 while in the background
        GEngine->SetMaxFPS(10.0f);
    }
#endif

    // Broadcast to any listening blueprints
    OnWindowFocusChanged.Broadcast(bIsFocused);
}
