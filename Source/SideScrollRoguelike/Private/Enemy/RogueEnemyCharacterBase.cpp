// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/RogueEnemyCharacterBase.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Enemy/RogueEnemyAIControllerBase.h"
#include "Engine/HitResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/RoguePlayerCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueEnemyCharacterBase)

void ARogueEnemyCharacterBase::BeginPlay()
{
    // Save the original max walk speed and disable movement, we only want to enable it when the player is in the patrol volume.
    GetCharacterMovement()->DisableMovement();
    InitialMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    Super::BeginPlay();

    /**
     * As part of BeginPlay here, we have to check if the player is already overlapping our trigger volumes.
     * This is because a "Begin Overlap" event won't be dispatched if the player is spawned inside
     * when the level is started.
     */
    TArray<AActor*> OverlappingActors;

    if (IsValid(AttackTriggerVolume))
    {
        AttackTriggerVolume->GetOverlappingActors(OverlappingActors, ARoguePlayerCharacter::StaticClass());
        if (OverlappingActors.Num() > 0)
        {
            OnBeginAttackTriggerOverlap(AttackTriggerVolume, OverlappingActors[0], nullptr, 0, false, FHitResult());
        }
    }

    if (IsValid(PatrolTriggerVolume))
    {
        PatrolTriggerVolume->GetOverlappingActors(OverlappingActors, ARoguePlayerCharacter::StaticClass());
        if (OverlappingActors.Num() > 0)
        {
            OnBeginPatrolTriggerOverlap(PatrolTriggerVolume, OverlappingActors[0], nullptr, 0, false, FHitResult());
        }
    }
}

void ARogueEnemyCharacterBase::BeginDestroy()
{
    // Unbind the trigger volume binds
    if (IsValid(PatrolTriggerVolume))
    {
        PatrolTriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnBeginPatrolTriggerOverlap);
        PatrolTriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnEndPatrolTriggerOverlap);
    }
    if (IsValid(AttackTriggerVolume))
    {
        AttackTriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnBeginAttackTriggerOverlap);
        AttackTriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnEndAttackTriggerOverlap);
    }

    Super::BeginDestroy();
}

void ARogueEnemyCharacterBase::HitCharacter()
{
    Super::HitCharacter();

    if (IsDead())
    {
        return;
    }

    // If we have a stun duration specified, start a timer for that
    if (HitStunDuration > 0.0f)
    {
        // Send the blueprint event
        OnHitStun();

        // Pass this event through to the Controller so it can know about it for the behavior tree to use
        if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
        {
            EnemyController->OnHitStun();
        }

        // Set the timer to notify the character of when the stun timer has elapsed
        GetWorldTimerManager().SetTimer(TimerHandle_HitStun, this, &ThisClass::StopHitStun, HitStunDuration);
    }
}

void ARogueEnemyCharacterBase::HitBeginOverlap(AActor* OverlappedActor, float Force)
{
    // If this enemy is dead, ignore any hurt overlaps
    if (IsDead())
    {
        return;
    }

    // We only care if a player has overlapped here.
    if (ARoguePlayerCharacter* Player = Cast<ARoguePlayerCharacter>(OverlappedActor))
    {
        if (Player->IsDead())
        {
            return;
        }

        if (Force <= 0.0f)
        {
            Player->HitCharacter();
        }
        else
        {
            // Calculate a force based on actor positions and the magnitude passed in
            const FVector HitForce = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal() * Force;
            Player->HitCharacterWithLaunchForce(HitForce);
        }
    }
}

void ARogueEnemyCharacterBase::HurtBeginOverlap(AActor* OverlappedActor, UBoxComponent* HurtBox, float RecoilForce)
{
    // If this enemy is dead, ignore any hurt overlaps
    if (IsDead()) return;

    // We only care if a player has overlapped here.
    if (ARoguePlayerCharacter* Player = Cast<ARoguePlayerCharacter>(OverlappedActor))
    {
        // Check if the player can make a valid jump off the enemy based on their state and how they've entered the hurt box.
        if (!Player->IsEnemyJumpValid(HurtBox))
        {
            return;
        }

        if (RecoilForce > 0.0f)
        {
            // Apply an immediate impulse to the player, opposite of how they hit the enemy
            FVector HitForce = FVector::ZeroVector;
            HitForce.X       = Player->GetVelocity().GetSafeNormal().X * -RecoilForce;
            HitForce.Z       = -Player->GetVelocity().Z;

            // We override the XY velocity here to cancel out any velocity the character had
            Player->LaunchCharacter(HitForce, true, true); // XY Override, Z Override
        }

        // Have the player perform an enemy jump
        Player->JumpFromEnemyHurtBox();

        // Apply a hit to this enemy
        HitCharacter();
    }
}

FVector ARogueEnemyCharacterBase::GetNextPatrolLocation()
{
    // If we are a non-patrolling enemy, early out
    if (!IsValid(PatrolSpline)) return FVector::ZeroVector;

    IncrementPatrolPoint();

    // Get the world position of the spline point
    return PatrolSpline->GetLocationAtSplinePoint(CurrentPatrolPointIndex, ESplineCoordinateSpace::World);
}

void ARogueEnemyCharacterBase::IncrementPatrolPoint()
{
    // If we are a non-patrolling enemy, early out
    if (!IsValid(PatrolSpline)) return;

    // If we're at the end of the patrol spline, rollover
    if (CurrentPatrolPointIndex + 1 == PatrolSpline->GetNumberOfSplinePoints())
    {
        CurrentPatrolPointIndex = 0;
    }
    else
    {
        CurrentPatrolPointIndex++;
    }
}

void ARogueEnemyCharacterBase::SetMovementSpeedMultiplier(float NewMultiplier)
{
    SpeedMultiplier = NewMultiplier;

    GetCharacterMovement()->MaxWalkSpeed = InitialMaxWalkSpeed * SpeedMultiplier;
}

void ARogueEnemyCharacterBase::RevertMovementSpeedMultiplier()
{
    SetMovementSpeedMultiplier(1.0f);
}

void ARogueEnemyCharacterBase::Init(FRogueEnemyInitializationArgs InitArgs)
{
    if (auto* PrimitiveComponent = GetComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->BodyInstance.bLockXTranslation = InitArgs.LockXTransform;
        PrimitiveComponent->BodyInstance.bLockYTranslation = InitArgs.LockYTransform;
        PrimitiveComponent->BodyInstance.bLockZTranslation = InitArgs.LockZTransform;
        PrimitiveComponent->BodyInstance.bLockTranslation  = InitArgs.LockXTransform || InitArgs.LockYTransform || InitArgs.LockZTransform;
    }

    // Setup Patrol behavior
    PatrolSpline = InitArgs.PatrolSpline;

    if (IsValid(PatrolSpline))
    {
        // Run any blueprint specific patrol setup behavior
        OnSetupPatrol();
    }

    // Setup Trigger Volumes to send events
    PatrolTriggerVolume = InitArgs.PatrolTriggerBox;
    AttackTriggerVolume = InitArgs.AttackTriggerBox;

    if (IsValid(PatrolTriggerVolume))
    {
        PatrolTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginPatrolTriggerOverlap);
        PatrolTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndPatrolTriggerOverlap);
    }

    if (IsValid(AttackTriggerVolume))
    {
        AttackTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginAttackTriggerOverlap);
        AttackTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndAttackTriggerOverlap);
    }
}

float ARogueEnemyCharacterBase::GetMovementSpeedMultiplier()
{
    return SpeedMultiplier;
}

void ARogueEnemyCharacterBase::OnBeginPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ARoguePlayerCharacter>(Other))
    {
        // Enable movement
        GetCharacterMovement()->SetDefaultMovementMode();

        // Notify the blueprint of this event
        OnPlayerEnteredPatrolTriggerVolume();

        // Pass this event through to the Controller so it can know about it for the behavior tree to use
        if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
        {
            EnemyController->OnPlayerEnteredPatrolTriggerVolume();
        }
    }
}

void ARogueEnemyCharacterBase::OnEndPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<ARoguePlayerCharacter>(Other))
    {
        // Disable movement
        GetCharacterMovement()->DisableMovement();

        // Notify the blueprint of this event
        OnPlayerExitedPatrolTriggerVolume();

        // Pass this event through to the Controller so it can know about it for the behavior tree to use
        if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
        {
            EnemyController->OnPlayerExitedPatrolTriggerVolume();
        }
    }
}

void ARogueEnemyCharacterBase::OnBeginAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ARoguePlayerCharacter>(Other))
    {
        // Set our attack speed multiplier
        SetMovementSpeedMultiplier(AttackSpeedMultiplier);

        // Notify the blueprint of this event
        OnPlayerEnteredAttackTriggerVolume();

        // Pass this event through to the Controller so it can know about it for the behavior tree to use
        if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
        {
            EnemyController->OnPlayerEnteredAttackTriggerVolume();
        }
    }
}

void ARogueEnemyCharacterBase::OnEndAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<ARoguePlayerCharacter>(Other))
    {
        // Revert the speed to normal
        RevertMovementSpeedMultiplier();

        // Notify the blueprint of this event
        OnPlayerExitedAttackTriggerVolume();

        // Pass this event through to the Controller so it can know about it for the behavior tree to use
        if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
        {
            EnemyController->OnPlayerExitedAttackTriggerVolume();
        }
    }
}

void ARogueEnemyCharacterBase::StopHitStun()
{
    // Send the blueprint event
    OnStopHitStun();

    // Pass this event through to the Controller so it can know about it for the behavior tree to use
    if (ARogueEnemyAIControllerBase* EnemyController = GetEnemyController())
    {
        EnemyController->OnStopHitStun();
    }
}

ARogueEnemyAIControllerBase* ARogueEnemyCharacterBase::GetEnemyController() const
{
    return GetController<ARogueEnemyAIControllerBase>();
}