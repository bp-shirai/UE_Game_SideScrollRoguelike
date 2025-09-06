// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RogueCharacterBase.generated.h"


UCLASS(Abstract)
class SIDESCROLLROGUELIKE_API ARogueCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterDeath_Delegate);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterHit_Delegate);

    ARogueCharacterBase();

    //--- Begin ACharacter overrides 

    // Called when the character has landed on the ground
    virtual void Landed(const FHitResult &Hit) override;
    virtual void BeginPlay() override;

    //--- End ACharacter overrides 

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rogue|Character|State")
    bool IsDead() const { return CurrentHitPoints <= 0; }

    // Applies a hit to this character
    UFUNCTION(BlueprintCallable, Category = "Rogue|Character|Combat")
    virtual void HitCharacter();

    // Applies a hit with force to the character based on impact
    UFUNCTION(BlueprintCallable, Category = "Rogue|Character|Combat")
    virtual void HitCharacterWithLaunchForce(const FVector& Force);

    // Instantly kills this character regardless of current hit point count.
    UFUNCTION(BlueprintCallable, Category = "Rogue|Character|State")
    void KillCharacter();

    // Notified by the hit animation that the character's head is fully reeled back so we can play any hit VFX
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Rogue|Character|Combat")
    void OnAnimNotifyHitEffect_BP();

    // Delegate that is fired when the character dies
    UPROPERTY(BlueprintAssignable, Category = "Rogue|Character|Combat")
    FCharacterDeath_Delegate OnCharacterDeath;

    // Delegate that is fired when the character is hit/attacked
    UPROPERTY(BlueprintAssignable, Category = "Rogue|Character|Combat")
    FCharacterHit_Delegate OnCharacterHit;

protected:
    // This is the authored value of hit points this character will have.
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0"), Category = "Rogue|Character|Status")
    int32 HitPoints = 1;

    // This is the working hit point count that is updated when attacked.
    UPROPERTY(BlueprintReadOnly, Category = "Rogue|Character|Status")
    int32 CurrentHitPoints = 0;

    // C++ logic implementation for when the character dies
    UFUNCTION(BlueprintCallable, Category = "Rogue|Character|Combat")
    virtual void CharacterDeath();

    // Blueprint event here so our derived blueprint can do anything it needs to
    UFUNCTION(BlueprintImplementableEvent, Category = "Rogue|Character|Combat")
    void OnCharacterDeath_BP();

    // Event to be notified when we are hit during combat. Blueprint Implementable to keep the implementation easy to tweak/change by design.
    UFUNCTION(BlueprintImplementableEvent, Category = "Rogue|Character|Combat")
    void OnCharacterHit_BP();
};
