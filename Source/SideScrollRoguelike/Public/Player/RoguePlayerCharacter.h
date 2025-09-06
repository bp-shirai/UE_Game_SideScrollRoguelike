// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RogueCharacterBase.h"
#include "RoguePlayerCharacter.generated.h"

class ARogueEnemyCharacterBase;
class UBoxComponent;
class URogueCharacterMovementComponent;
class UCharacterMovementComponent;

/**
 *
 * This class builds on the default and Rogue Character implementation, overriding where necessary
 * to provide the functionality needed for our player.
 * Similarly, we also has a custom movement component: URogueCharacterMovementComponent
 */
UCLASS()
class SIDESCROLLROGUELIKE_API ARoguePlayerCharacter : public ARogueCharacterBase
{
    GENERATED_BODY()

public:
    // Hit point added delegate
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitpointsAdded);

    // When true, the player wants to jump
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rogue|Input")
    bool IsJumpInputActive() const { return bPressedJump; }

    //--- ACharacter overrides

    // Stops the character from jumping on the next update.
    // Call this from an input event (such as a button 'up' event) to cease applying the jump.
    virtual void StopJumping() override;

    // Function that is called on tick of the character movement component.
    // Triggers jump if the jump button has been pressed.
    virtual void CheckJumpInput(float DeltaTime) override;

    /**
     * Customizable event to check if the character can jump in the current state.
     * The reason that this override has _Implementation here is because this is a blueprint native event.
     * So there is a native C++ implementation and a blueprint implementation. For more information on how this implementation works,
     * see the documentation here: https://dev.epicgames.com/documentation/en-us/unreal-engine/implementing-blueprint-interfaces-in-unreal-engine#blueprintnativeevents
     *
     * @Return Whether the character can jump in the current state.
     */
    virtual bool CanJumpInternal_Implementation() const override;

    //--- End ACharacter overrides

    //--- ARogueCharacterBase overrides

    // Hits this character
    virtual void HitCharacter() override;

    // Hits this character with a specified force
    virtual void HitCharacterWithLaunchForce(const FVector& Force) override;

    //--- End ARogueCharacterBase overrides

    // Returns true when the player can make a valid jump off of the overlapped hurt box given the sweep result
    UFUNCTION(BlueprintCallable)
    bool IsEnemyJumpValid(UBoxComponent* HurtBox);

    // Executes a jump from an enemy's hurt box
    void JumpFromEnemyHurtBox();

public:
    // Delegate for when hit points are added to the player's health
    UPROPERTY(BlueprintAssignable)
    FOnHitpointsAdded OnHitpointsAdded;

protected:
    // A duration to stun the player when hit. Zero will prevent any stun
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0", Category = "Rogue|Character|Stats"))
    float HitStunDuration = 0.0f;

    // When true, allows the player to be stunned in mid-air
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (EditConditionHides, EditCondition = "HitStunDuration > 0", Category = "Rogue|Character|Stats"))
    bool bStunMidAir = false;

    // A duration that the player is invulnerable after being hit
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0", Category = "Rogue|Character|Stats"))
    float HitInvulnerabilityDuration = 0.0f;

    // Tracks whether or not the speed powerup is active
    UPROPERTY(BlueprintReadOnly, Category = "Rogue|Player|Powerups")
    bool bIsSpeedPowerupActive;

    // Handle for efficient management of the speed power up timer.
    // Timer Handles are used to edit a running timer. See ARoguePlayerCharacter::ActivateSpeedPowerup for an example use case.
    // Official Timer documentation covering handles here: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-timers-in-unreal-engine?application_version=5.5
    FTimerHandle TimerHandle_SpeedPowerup;

    // Handle for efficient management of the hit stun timer.
    FTimerHandle TimerHandle_HitStun;

    // Handle for efficient management of the hit invulnerability timer
    FTimerHandle TimerHandle_HitInvulnerability;

    // Store the initial max acceleration set by the Character Movement Component in Blueprint
    float InitialMaxAcceleration;

    // Store the initial max walk speed set by the Character Movement Component in Blueprint
    float InitialMaxWalkSpeed;

    // When true, the player is invulnerable to incoming hits
    bool bHitInvulnerable;

    // The default collision response type for the enemy collision object channel
    ECollisionResponse DefaultEnemyCollisionResponseType;

    // The default collision response type for cannonball collision object channel
    ECollisionResponse DefaultCannonballCollisionResponseType;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Overridden from RogueCharacterBase
    virtual void CharacterDeath() override;

    // Called when the speed powerup timer has completed.
    void StopSpeedPowerup();

    // Called when the hit stun timer has completed
    void StopHitStun();

    // Called when the hit invulnerability timer has completed
    void StopHitInvulnerability();

    // Adds a fixed amount of hit points to character's current hit points
    UFUNCTION(BlueprintCallable, Category = "Rogue|Player|Powerups")
    void AddHitpoints(int32 PointsToAdd);

    // Modifies Character Movement Component using a timer for the passed duration
    UFUNCTION(BlueprintCallable, Category = "Rogue|Player|Powerups")
    void ActivateSpeedPowerup(float Duration, float MaxSpeedMultiplier);

    // Called when the speed powerup has been activated
    UFUNCTION(BlueprintImplementableEvent, Category = "Rogue|Player|Powerups", meta = (DisplayName = "OnSpeedPowerupActivated"))
    void OnSpeedPowerupActivated_BP(float Duration);

    // Called when a player jumps off of an enemy
    UFUNCTION(BlueprintImplementableEvent, Category = "Rogue|Player|State", meta = (DisplayName = "OnEnemyJump"))
    void OnEnemyJump_BP();

public:

    URogueCharacterMovementComponent* GetRogueCharacterMovementComponent() const;
};
