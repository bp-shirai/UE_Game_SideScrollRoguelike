// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/RogueCharacterBase.h"

#include "Engine/EngineTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCharacterBase)

// Sets default values
ARogueCharacterBase::ARogueCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARogueCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHitPoints = HitPoints;
}

void ARogueCharacterBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // If the character is dead and was in the air, we need to stop the character's movement when we land
    // This allows the character's mesh to behave appropriately to land on a platform or the landscape
    // before movement is halted.
    if (IsDead())
    {
        switch (Hit.Component->GetCollisionObjectType())
        {
        case ECollisionChannel::ECC_WorldStatic:
        case ECollisionChannel::ECC_WorldDynamic:
            if (GetCharacterMovement()->IsMovingOnGround())
            {
                GetCharacterMovement()->DisableMovement();
                GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            break;
        }
    }
}

void ARogueCharacterBase::CharacterDeath()
{
    // We want to check if the character is on the ground when death occurs
    // This is so we can disable the movement entirely so the corpse doesn't
    // move around/fall/etc.
    //
    // If the character is in the air, we don't disable it now. We listen in Landed()
    // so we can disable movement once the character has fallen down.
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // We want to play the hit effect if the character dies. The hit animation
    // won't play to trigger the anim notify, so we call it manually.
    OnAnimNotifyHitEffect_BP();

    // Invoke the BlueprintImplementable Event so the derived blueprint class knows
    OnCharacterDeath_BP();
    // Broadcast the delegate so any subscribed classes or blueprints know
    OnCharacterDeath.Broadcast();
}

void ARogueCharacterBase::HitCharacter()
{
    // If a hit event is invoke after the character has died, ignore this call
    if (IsDead()) return;

    CurrentHitPoints = (CurrentHitPoints > 0) ? CurrentHitPoints - 1 : 0;

    OnCharacterHit_BP();
    OnCharacterHit.Broadcast();

    if (IsDead())
    {
        CharacterDeath();
    }
}

void ARogueCharacterBase::HitCharacterWithLaunchForce(const FVector& Force)
{
    if (IsDead()) return;

    // Apply an impulse force to the character
    FVector LaunchVelocity = Force;

    // Character should never move along the Y-axis so we zero that out here just in case
    LaunchVelocity.Y = 0.0f;

    // We override th XY and Z velocity here cancel out any velocity the character had
    LaunchCharacter(LaunchVelocity, true, true);

    HitCharacter();
}

void ARogueCharacterBase::KillCharacter()
{
    if (!IsDead())
    {
        CurrentHitPoints = 0;
        CharacterDeath();
    }
}
