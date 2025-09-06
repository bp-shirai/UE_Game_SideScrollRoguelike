// Fill out your copyright notice in the Description page of Project Settings.

#include "Audio/RogueAudioSubsystem.h"

#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Engine/WorldInitializationValues.h"
#include "Game/RogueGameInstance.h"
#include "Game/RogueGameState.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingScreenManager.h"
#include "PlatformFeatures.h"
#include "Settings/RogueDeveloperSettings.h"
#include "Settings/RogueGameUserSettings.h"
#include "Settings/RogueWorldSettings.h"

#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogRogueAudioSubsystem);

void URogueAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Ask the loading screen to notify us when its visibility changes
    ULoadingScreenManager* LoadingScreenManager = Collection.InitializeDependency<ULoadingScreenManager>();
    LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().AddUObject(this, &ThisClass::LoadingScreenVisibilityChanged);

    // Get a reference to our game instance
    RogueGameInstance = Cast<URogueGameInstance>(GetGameInstance());

    // Get the class default object for Rogue Developer Settings
    CurrentDeveloperSettings = GetDefault<URogueDeveloperSettings>();

    // Load default audio classes from the Rogue Developer Project Settings
    // Note that we're using IsNull here to check if the soft object pointer path is null
    // IsValid, checks an already loaded object

    // Default Sound Mixer
    if (!CurrentDeveloperSettings->DefaultSoundMixModifier.IsNull())
    {
        DefaultSoundMixModifier = CurrentDeveloperSettings->DefaultSoundMixModifier.LoadSynchronous();
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::Initialize DefaultSoundMixModifier is invalid, check your Rogue Developer settings."));
    }

    // Main Sound
    if (!CurrentDeveloperSettings->MainSoundClass.IsNull())
    {
        MainSoundClass = CurrentDeveloperSettings->MainSoundClass.LoadSynchronous();
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::Initialize MainSoundClass is invalid, check your Rogue Developer settings."));
    }

    // Music
    if (!CurrentDeveloperSettings->MusicSoundClass.IsNull())
    {
        MusicSoundClass = CurrentDeveloperSettings->MusicSoundClass.LoadSynchronous();
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::Initialize MusicSoundClass is invalid, check your Rogue Developer settings."));
    }

    // SFX
    if (!CurrentDeveloperSettings->SFXSoundClass.IsNull())
    {
        SFXSoundClass = CurrentDeveloperSettings->SFXSoundClass.LoadSynchronous();
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::Initialize SFXSoundClass is invalid, check your Rogue Developer settings."));
    }

    // Load the (optional) default music for the game from the Rogue Developer Project Settings
    if (!CurrentDeveloperSettings->LevelCompleteMusic.IsNull())
    {
        LevelCompleteMusic = CurrentDeveloperSettings->LevelCompleteMusic.LoadSynchronous();
    }

    if (!CurrentDeveloperSettings->LevelFailMusic.IsNull())
    {
        LevelFailMusic = CurrentDeveloperSettings->LevelFailMusic.LoadSynchronous();
    }

    // Bind to world initialization so that the audio subsystem knows about the world state (BeginPlay)
    PostWorldCreationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &URogueAudioSubsystem::WorldInitialization);
}

void URogueAudioSubsystem::SaveAudioSettings()
{
    if (CurrentGameSettings)
    {
        CurrentGameSettings->SaveSettings();
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::SaveAudioSettings CurrentGameSettings is nullptr."));
        return;
    }
}

void URogueAudioSubsystem::SetMainVolume(float NewVolume, float FadeIn)
{
    if (CurrentGameSettings)
    {
        CurrentGameSettings->MainVolume = NewVolume;

        ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, FadeIn);
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::SetMainVolume CurrentGameSettings is nullptr."));
        return;
    }
}

void URogueAudioSubsystem::SetMusicVolume(float NewVolume, float FadeIn)
{
    if (CurrentGameSettings)
    {
        CurrentGameSettings->MusicVolume = NewVolume;

        ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, FadeIn);
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::SetMusicVolume CurrentGameSettings is nullptr."));
        return;
    }
}

void URogueAudioSubsystem::SetSFXVolume(float NewVolume, float FadeIn)
{
    if (CurrentGameSettings)
    {
        CurrentGameSettings->SFXVolume = NewVolume;

        ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, FadeIn);
    }
    else
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::SetSFXVolume CurrentGameSettings is nullptr."));
        return;
    }
}

float URogueAudioSubsystem::GetMainVolume()
{
    if (CurrentGameSettings)
    {
        return CurrentGameSettings->MainVolume;
    }

    UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::GetMainVolume CurrentGameSettings is nullptr."));
    return 1.0f;
}

float URogueAudioSubsystem::GetMusicVolume()
{
    if (CurrentGameSettings)
    {
        return CurrentGameSettings->MusicVolume;
    }

    UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::GetMusicVolume CurrentGameSettings is nullptr."));
    return 1.0f;
}

float URogueAudioSubsystem::GetSFXVolume()
{
    if (CurrentGameSettings)
    {
        return CurrentGameSettings->SFXVolume;
    }

    UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::GetSFXVolume CurrentGameSettings is nullptr."));
    return 1.0f;
}

bool URogueAudioSubsystem::WorldMusicInitialized()
{
    if (!IsValid(WorldMusicPlayer))
    {
        return false;
    }

    return true;
}

void URogueAudioSubsystem::PlaySoundAsWorldMusic(USoundBase* Music)
{
    PlayWorldMusic(Music);
}

void URogueAudioSubsystem::ApplyVolumeChangeToMix(USoundClass* TargetSoundClass, float Volume, float FadeIn)
{
    // Get the audio device, apply the override to the mix, push modifier update
    if (FAudioDeviceHandle AudioDevice = CurrentWorld->GetAudioDevice())
    {
        AudioDevice->SetSoundMixClassOverride(
            DefaultSoundMixModifier, /* Sound Mix Modifier */
            TargetSoundClass,        /* Sound Class */
            Volume,                  /* Volume Multiplier*/
            1.0f,                    /* Pitch Multiplier */
            FadeIn,                  /* Fade In Time */
            true                     /* Apply To Children */
        );
        AudioDevice->PushSoundMixModifier(DefaultSoundMixModifier);
    }
}

void URogueAudioSubsystem::WorldInitialization(UWorld* World, const FWorldInitializationValues IVS)
{
    if (World)
    {
        // The world has been initialized so now we can bind to BeginPlay of the world.
        // Here, we bind our world begin play function to this delegate.
        CurrentWorld           = World;
        OnWorldBeginPlayHandle = CurrentWorld->OnWorldBeginPlay.AddUObject(this, &ThisClass::WorldBeginPlay);

        // Store the current world settings for pulling relevant audio data
        CurrentWorldSettings = Cast<ARogueWorldSettings>(CurrentWorld->GetWorldSettings());

        if (!CurrentWorldSettings)
        {
            UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::WorldInitialization unable to get Rogue world settings. Check world %s"), *CurrentWorld->GetName());
            return;
        }
    }
}

void URogueAudioSubsystem::WorldBeginPlay()
{
    // Get the game user settings
    CurrentGameSettings = GetRogueGameSettings();

    // Apply sound settings to world audio device
    UpdateMixersFromAudioData();

    // Optionally bind to Rogue game state so the subsystem is informed of state events
    if ((RogueGameState = CurrentWorld->GetGameState<ARogueGameState>()))
    {
        RogueGameState->OnLevelStateChanged.AddDynamic(this, &ThisClass::GameStateChanged);
    }

    // In editor, we want to play the music on world begin play
#if WITH_EDITOR
    // If the loading screen is held, the loading screen will notify us via LoadingScreenVisibilityChanged
    // If we are changing levels with no loading screen, just start the music
    if (IsValid(RogueGameInstance) && !RogueGameInstance->ShouldHoldLoadingScreen())
    {
        StartDefaultWorldMusic();
    }
#endif
}

void URogueAudioSubsystem::LoadingScreenVisibilityChanged(bool bVisible)
{
    if (!bVisible)
    {
        // Now that the loading screen is no longer visible, play the world music
        StartDefaultWorldMusic();
    }
    else
    {
        // When a loading screen has popped up, stop any world music
        if (IsValid(WorldMusicPlayer))
        {
            WorldMusicPlayer->Stop();
        }
    }
}

void URogueAudioSubsystem::UpdateMixersFromAudioData()
{
    if (IsValid(CurrentGameSettings))
    {
        // Apply audio data settings to mixer
        ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, 0.0f);
        ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, 0.0f);
        ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, 0.0f);
    }
}

void URogueAudioSubsystem::GameStateChanged(ELevelState NewLevelState)
{
    if (NewLevelState == ELevelState::Paused || NewLevelState == ELevelState::Running)
    {
        TogglePauseWorldMusic();
        return;
    }

    if (NewLevelState == ELevelState::GameOver)
    {
        StartGameOverMusic();
        return;
    }

    if (NewLevelState == ELevelState::Victory)
    {
        StartVictoryMusic();
        return;
    }
}

void URogueAudioSubsystem::StartDefaultWorldMusic()
{
    if (!IsValid(CurrentWorld))
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::StartDefaultWorldMusic CurrentWorld is null"));
        return;
    }

    if (!IsValid(CurrentWorldSettings))
    {
        UE_LOG(LogRogueAudioSubsystem, Error, TEXT("URogueAudioSubsystem::StartDefaultWorldMusic CurrentWorldSettings is null"));
        return;
    }

    if (!IsValid(CurrentWorldSettings->WorldMusic))
    {
        UE_LOG(LogRogueAudioSubsystem, Warning, TEXT("URogueAudioSubsystem::StartDefaultWorldMusic WorldMusic is null. Check your world settings setup %s"), *CurrentWorld->GetName());
        return;
    }

    PlayWorldMusic(CurrentWorldSettings->WorldMusic);
}

void URogueAudioSubsystem::PlayWorldMusic(USoundBase* Music)
{
    if (!IsValid(Music))
    {
        UE_LOG(LogRogueAudioSubsystem, Warning, TEXT("URogueAudioSubsystem::PlayWorldMusic, Music parameter is null."));
        return;
    }

    if (!IsValid(WorldMusicPlayer))
    {
        // Spawn our audio component
        WorldMusicPlayer = UGameplayStatics::CreateSound2D(
            CurrentWorld, /* World Object */
            Music,        /* USoundBase* */
            1.0f,         /* Volume Multiplier */
            1.0f,         /* Pitch Multiplier */
            0.0f,         /* Start Time */
            nullptr,      /* Concurrency Settings */
            true,         /* Persist across level transition */
            false);       /* Auto Destroy */

        if (CurrentGameSettings)
        {
            // Note that the reason we don't set the multiplier directly on the audio component is because
            // the audio data multiplier is applied to the mix the sound is apart of, not the player itself.
            SetMusicVolume(CurrentGameSettings->MusicVolume);
        }
    }

    if (WorldMusicPlayer->bIsPaused)
    {
        WorldMusicPlayer->SetPaused(false);
    }

    if (WorldMusicPlayer->IsPlaying())
    {
        WorldMusicPlayer->Stop();
    }

    // Switch the sound
    WorldMusicPlayer->Sound = Music;

    // Play the music from the beginning
    WorldMusicPlayer->Play();
}

void URogueAudioSubsystem::TogglePauseWorldMusic()
{
    if (!IsValid(WorldMusicPlayer))
    {
        return;
    }

    // Toggle pause of the music player
    WorldMusicPlayer->SetPaused(!WorldMusicPlayer->bIsPaused);
}

void URogueAudioSubsystem::StartVictoryMusic()
{
    if (!IsValid(WorldMusicPlayer))
    {
        return;
    }

    WorldMusicPlayer->Stop();

    if (!IsValid(LevelCompleteMusic))
    {
        // No music found so we'll just stop the music player and return
        return;
    }

    // Set the new sound and play
    WorldMusicPlayer->SetSound(LevelCompleteMusic);
    WorldMusicPlayer->Play();
}

void URogueAudioSubsystem::StartGameOverMusic()
{
    if (!IsValid(WorldMusicPlayer))
    {
        return;
    }

    WorldMusicPlayer->Stop();

    if (!IsValid(LevelFailMusic))
    {
        // No music found so we'll just stop the music player and return
        return;
    }

    // Set the new sound and play
    WorldMusicPlayer->SetSound(LevelFailMusic);
    WorldMusicPlayer->Play();
}

TObjectPtr<URogueGameUserSettings> URogueAudioSubsystem::GetRogueGameSettings()
{
    if (GEngine)
    {
        return Cast<URogueGameUserSettings>(GEngine->GetGameUserSettings());
    }

    return nullptr;
}

UObject* URogueAudioSubsystem::LoadSoftObjectPtrSynchronous(TSoftObjectPtr<UObject> SoftObjectPtr)
{
    if (SoftObjectPtr.IsValid())
    {
        return SoftObjectPtr.Get();
    }

    UObject* LoadedAsset = SoftObjectPtr.LoadSynchronous();
    if (!LoadedAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to synchronously load asset: %s"), *SoftObjectPtr.ToString());
    }

    return LoadedAsset;
}
