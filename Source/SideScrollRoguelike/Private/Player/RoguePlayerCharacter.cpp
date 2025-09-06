// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/RoguePlayerCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/RogueEnemyCharacterBase.h"
#include "Game/RogueGameState.h"
#include "GameFramework/PlayerController.h"
#include "Math/MathFwd.h"
#include "Math/UnrealMathUtility.h"
#include "Player/RogueCharacterMovementComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlayerCharacter)

void ARoguePlayerCharacter::StopJumping()
{
    // Inform our movement component that the jump input has stopped
    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        MoveComp->StopJumpInput();
    }

    // Now we can reset our jump state as normal just like ACharacter::StopJumping()
    ResetJumpState();
}

void ARoguePlayerCharacter::CheckJumpInput(float DeltaTime)
{
    // This function is almost identical to ACharacter::CheckJumpInput.
    // The difference is that we do not increment the jump count an additional time when the player is already falling.
    // This is necessary for recognizing coyote time jumps as the first jump.

    JumpCurrentCountPreJump = JumpCurrentCount;

    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        if (bPressedJump)
        {
            const bool bDidJump = CanJump() && MoveComp->DoJump(bClientUpdating, DeltaTime);
            if (bDidJump)
            {
                // Transition from not (actively) jumping to jumping.
                if (!bWasJumping)
                {
                    JumpCurrentCount++;
                    JumpForceTimeRemaining = GetJumpMaxHoldTime();
                    OnJumped();
                }
            }
            bWasJumping = bDidJump;
        }
    }
}

bool ARoguePlayerCharacter::CanJumpInternal_Implementation() const
{
    // Can't jump while dead
    if (IsDead()) return false;

    // The rest of this function is almost identical to ACharacter::CanJumpInternal_Implementation.
    // The difference is that we do not special case for the first jump when the player is already falling.
    // This is necessary for recognizing coyote time jumps as the first jump.
    URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent();

    bool bJumpIsAllowed = MoveComp ? MoveComp->CanAttemptJump() : false;

    // Ensure JumpHoldTime and JumpCount are valid.
    if (!bWasJumping || GetJumpMaxHoldTime() <= 0.f)
    {
        bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
    }
    else
    {
        // Only consider JumpKeyHoldTime as long as:
        // A) The jump limit hasn't been met OR
        // B) The jump limit has been met AND we were already jumping
        const bool bJumpKeyHold = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());

        bJumpIsAllowed = bJumpKeyHold && ((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
    }

    return bJumpIsAllowed;
}

bool ARoguePlayerCharacter::IsEnemyJumpValid(UBoxComponent* HurtBox)
{
    // When the player is dead, they can't jump off an enemy
    if (IsDead()) return false;

    // If the character isn't falling on Z, we cannot jump off an enemy
    // This prevents cases where a player could intersect the hurt box while ascending
    if (GetCharacterMovement()->Velocity.Z > 0.f) return false;

    const FVector& CurrentLocation = GetActorLocation();
    const FVector& BoxExtents      = HurtBox->GetScaledBoxExtent();
    const FVector& BoxCenter       = HurtBox->GetComponentLocation();

    // We subtract here just to add a buffer around floating point precision
    const float BoxUpperBoundZ = (BoxCenter.Z + BoxExtents.Z) - UE_KINDA_SMALL_NUMBER;

    // Character location must be above the upper bound Z for a valid jump
    if (CurrentLocation.Z < BoxUpperBoundZ)
    {
        return false;
    }

    return true;
}

void ARoguePlayerCharacter::JumpFromEnemyHurtBox()
{
    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        // Perform the jump on the character movement
        MoveComp->DoEnemyJump();

        // Broadcast to blueprint
        OnEnemyJump_BP();
    }
}

void ARoguePlayerCharacter::BeginPlay()
{
    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        // Store our starting values as these values can change when powerups are used
        InitialMaxAcceleration = MoveComp->MaxAcceleration;
        InitialMaxWalkSpeed    = MoveComp->MaxWalkSpeed;
    }

    // ECC_GameTraceChannel2 is cannonball.
    // ECC_GameTraceChannel3 is enemy.
    // Both of these object types can be seen in the DefaultEngine.ini which was set in the project's collision settings.
    // Custom collision object types can be set in that file or in editor under Edit/Project Settings/Collision
    DefaultCannonballCollisionResponseType = GetCapsuleComponent()->GetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2);
    DefaultEnemyCollisionResponseType      = GetCapsuleComponent()->GetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3);

    Super::BeginPlay();
}

void ARoguePlayerCharacter::HitCharacter()
{
    if (bHitInvulnerable) return;

    Super::HitCharacter();

    // If the character has died after a hit, we don't want to apply an timers
    if (IsDead()) return;

    // Do not allow mid air stuns if the toggle is set to false
    if (!bStunMidAir && GetCharacterMovement()->IsFalling()) return;

    // If we have a stun duration specified, start a timer for that
    if (HitStunDuration > 0.0f)
    {
        // Apply any changes we would like at the beginning of a stun
        // In this case, we're making the player immune to incoming hits and disabling input
        if (APlayerController* PlayerController = GetController<APlayerController>())
        {
            PlayerController->DisableInput(PlayerController);
        }

        // Timer handles are used to edit a timer. An example would be pausing a running timer.
        GetWorldTimerManager().SetTimer(TimerHandle_HitStun, this, &ThisClass::StopHitStun, HitStunDuration);
    }

    // If we have a hit invulnerability duration specified, start a timer for that
    if (HitInvulnerabilityDuration > 0.0f)
    {
        bHitInvulnerable = true;
        GetWorldTimerManager().SetTimer(TimerHandle_HitInvulnerability, this, &ThisClass::StopHitInvulnerability, HitInvulnerabilityDuration);
    }
}

void ARoguePlayerCharacter::HitCharacterWithLaunchForce(const FVector& Force)
{
    if (bHitInvulnerable)
    {
        return;
    }

    Super::HitCharacterWithLaunchForce(Force);
}

void ARoguePlayerCharacter::CharacterDeath()
{
    // First disable the player's input on the controller
    if (APlayerController* PlayerController = GetController<APlayerController>())
    {
        // The DisableInput function on AActor accepts an optional PlayerController parameter. It will disable
        // input on the given controller, or if none is provided it will disable input on every controller in the world.
        // We only want to disable this pawn's controller input in this case.
        PlayerController->DisableInput(PlayerController);
    }

    // If the character dies mid-air, we should stop any jump behavior
    if (JumpCurrentCount > 0)
    {
        StopJumping();
    }

    // If the character is ascending on Z and then dies, we should clear that velocity here so gravity takes over
    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        if (MoveComp->Velocity.Z > 0.f)
        {
            MoveComp->Velocity.Z = 0.f;
        }
    }

    // Inform our game state that this character has died
    if (ARogueGameState* RogueGameState = GetWorld()->GetGameState<ARogueGameState>())
    {
        RogueGameState->PlayerDeath(this);
    }

    Super::CharacterDeath();
}

void ARoguePlayerCharacter::AddHitpoints(int32 PointsToAdd)
{
    CurrentHitPoints += PointsToAdd;
    OnHitpointsAdded.Broadcast();
}

void ARoguePlayerCharacter::ActivateSpeedPowerup(float Duration, float MaxSpeedMultiplier)
{
    // Update our powerup state bool
    bIsSpeedPowerupActive = true;

    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        // Multiply both max acceleration and speed by our powerup multiplier
        MoveComp->MaxAcceleration = InitialMaxAcceleration * MaxSpeedMultiplier;
        MoveComp->MaxWalkSpeed    = InitialMaxWalkSpeed * MaxSpeedMultiplier;
    }

    // Toggle hit invulnerability for any hit events that could overlap this player
    bHitInvulnerable = true;

    // Turn collision off on a player capsule for enemy object types
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECR_Ignore);

    // Turn collision off on a player capsule for cannonball object types
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECR_Ignore);

    // Start a timer for the powerup
    GetWorldTimerManager().SetTimer(TimerHandle_SpeedPowerup, this, &ThisClass::StopSpeedPowerup, Duration);

    // Note that we did not check bIsSpeedPowerupActive here to see if a timer is already active. This is an important case to cover for sequential pickups.
    // We can avoid this because SetTimer will override whatever is set on TimerHandle_SpeedPowerup. Therefore each call to this function will reset our timer as expected.
    // For more information on Timers, see the official documentation: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-timers-in-unreal-engine?application_version=5.3

    // Emit to blueprint that our speed powerup has been activated
    OnSpeedPowerupActivated_BP(Duration);
}

void ARoguePlayerCharacter::StopSpeedPowerup()
{
    // Restore initial movement values

    if (URogueCharacterMovementComponent* MoveComp = GetRogueCharacterMovementComponent())
    {
        // Multiply both max acceleration and speed by our powerup multiplier
        MoveComp->MaxAcceleration = InitialMaxAcceleration;
        MoveComp->MaxWalkSpeed    = InitialMaxWalkSpeed;
    }

    // Toggle the powerup state tracking
    bIsSpeedPowerupActive = false;

    // Turn hit invulnerability off for any hit events that could overlap this player
    bHitInvulnerable = false;

    // Turn collision on for a player capsule for enemy object types
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, DefaultEnemyCollisionResponseType);

    // Turn collision on for a player capsule for enemy object types
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, DefaultCannonballCollisionResponseType);
}

void ARoguePlayerCharacter::StopHitStun()
{
    // Note that we don't need to clear the timer here as it is not set to looping. 
    if (APlayerController* PlayerController  =GetController<APlayerController>())
    {
        PlayerController->EnableInput(PlayerController);
    }
}

void ARoguePlayerCharacter::StopHitInvulnerability()
{
    // Note that we don't need to clear the timer here as it is not set to looping. 

    bHitInvulnerable = false;
}

URogueCharacterMovementComponent* ARoguePlayerCharacter::GetRogueCharacterMovementComponent() const
{
    return GetCharacterMovement<URogueCharacterMovementComponent>();
}
