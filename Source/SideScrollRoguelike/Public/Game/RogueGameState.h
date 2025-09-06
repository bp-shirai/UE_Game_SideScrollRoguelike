// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RogueGameState.generated.h"

class ARoguePlayerCharacter;

// This enum tracks the simple state of what is happening in the level.
UENUM(BlueprintType)
enum class ELevelState : uint8
{
    Preload,
    Ready,
    Running,
    Paused,
    GameOver,
    Victory
};

// Declare a delegate type with one parameter which will pass the new level state that we just transitioned to.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLevelStateChanged, ELevelState, NewLevelState);
// Declare a delegate type which will broadcast when the game state is initialized
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateInitialized);

/**
 *
 */
UCLASS(Abstract)
class SIDESCROLLROGUELIKE_API ARogueGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ARogueGameState();

    // Updates the game state when a player completes a level
    UFUNCTION(BlueprintCallable)
    void CompleteLevel(ARoguePlayerCharacter* Player);

    // Updates the game state when a player passes out of bounds
    UFUNCTION(BlueprintCallable)
    void PlayerOutOfBounds(ARoguePlayerCharacter* Player);

    // Updates the game state when a player is killed by enemy combat
    UFUNCTION(BlueprintCallable)
    void PlayerDeath(ARoguePlayerCharacter* Player);

    // Updates the game state when the final boss is killed
    UFUNCTION(BlueprintCallable)
    void BossDefeated(UPARAM(meta = (ClampMin = "0")) float Delay);

    // The function we call
    UFUNCTION()
    void BossDefeatedTimerComplete();

    // Handles level state changes and updates the cached level state
    void SetLevelState(ELevelState NewState);

    // State handling for ready state entry. When the game is initialized but not yet started.
    void HandleReady();

    // Pauses the level state
    UFUNCTION(BlueprintCallable)
    void PauseGame();

    // Unpauses the level state
    UFUNCTION(BlueprintCallable)
    void UnPauseGame();

    // Handles logic for when the level is reset.
    UFUNCTION(BlueprintCallable)
    virtual void ResetCurrentLevel();

    // When the game state has initialized
    UPROPERTY(BlueprintAssignable)
    FGameStateInitialized OnGameStateInitialized;

    // Our delegate instance. BlueprintAssignable here exposes this delegate to blueprints
    // When the level state has changed
    UPROPERTY(BlueprintAssignable)
    FLevelStateChanged OnLevelStateChanged;

protected:
    // Called by the game mode when play has started
    virtual void HandleBeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Helper function for checking if the level has ended
    virtual bool HasMatchEnded() const override;

protected:
    // The time in seconds that a player is allowed to complete the level before timing out
    // This must be set for the game state to function properly
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = 0.0, UIMin = 0.0, Category = "Parrot Game State"))
    float TimePerLevel;

    // The amount of time left remaining since the player started the level
    UPROPERTY(Transient, BlueprintReadOnly)
    float RemainingTime;

    // The amount of time accumulated since the player started the level
    UPROPERTY(Transient, BlueprintReadOnly)
    float AccumulatedTime;

    // The current simple state of the level
    UPROPERTY(Transient, BlueprintReadOnly)
    ELevelState LevelState = ELevelState::Preload;

private:
    // Handle for the timer we set in BossDefeated()
    FTimerHandle TimerHandle_BossDefeatedDelay;
};
