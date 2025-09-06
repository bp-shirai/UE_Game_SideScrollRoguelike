// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RogueCharacterBase.h"
#include "Enemy/RogueEnemyTypes.h"
#include "RogueEnemyCharacterBase.generated.h"

class USplineComponent;
//class UBoxComponent;
class ARogueEnemyAIControllerBase;




/**
 * The base class for our enemy types. 
 * Derives from ARogueCharacterBase and shares base functionality. 
 * Overrides functionality where appropriate for enemy specific behavior. 
 */
UCLASS()
class SIDESCROLLROGUELIKE_API ARogueEnemyCharacterBase : public ARogueCharacterBase
{
	GENERATED_BODY()
	
protected:

	// The default knock back force to apply when this enemy inflicts a hit on a player
	// Typically applied from capsule collisions
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0", Category = "Enemy|Combat"))
	float KnockbackForce = 500.0f; 

	// The speed multiplier applied when the player enters the attack volume of the enemy
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0", Category = "Enemy|Combat"))
	float AttackSpeedMultiplier = 4.0f;

	// A duration to stun the enemy when hit. Zero will prevent any stun
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0", Category = "Rogue|Character|Stats"))
	float HitStunDuration = 0.0f; 

	// The current attacking state of the enemy, used to drive animations
	UPROPERTY(BlueprintReadWrite, Category = "Enemy|Combat")
	bool IsAttacking = false;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy|Combat")
	bool IsShakingHead = false;

	// The spline which the enemy will patrol 
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Patrol")
	TObjectPtr<USplineComponent> PatrolSpline = nullptr;

	// Used to determine if the player is in a patrol zone
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Patrol")
	TObjectPtr<UBoxComponent> PatrolTriggerVolume = nullptr;

	// Used to determine if the player is in an attack zone
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Patrol")
	TObjectPtr<UBoxComponent> AttackTriggerVolume = nullptr;

protected:
	// Called when begin play starts for this character 
	virtual void BeginPlay() override;

	// Called when the actor is destroyed
	virtual void BeginDestroy() override;

	// Overridden from RogueCharacterBase
	// Called when this character incurs a hit 
	virtual void HitCharacter() override;

	// Invoked whenever an actor enters a hit collision by this enemy
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	void HitBeginOverlap(AActor* OverlappedActor, UPARAM(meta = (ClampMin = "0")) float Force = 0.0f);

	// Invoked whenever an actor enters a hurt collision with this enemy 
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	void HurtBeginOverlap(AActor* OverlappedActor, UBoxComponent* Hurtbox, UPARAM(meta = (ClampMin = "0")) float RecoilForce = 0.0f);

	// Called by the AI controller to get the next patrol location for patrolling enemies
	UFUNCTION(BlueprintCallable, Category = "Enemy|Patrol")
	FVector GetNextPatrolLocation();

	// Helper function to rollover the index if we reach the end of the spline
	void IncrementPatrolPoint();

	// Sets a multiplier that is applied to the enemy speed
	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void SetMovementSpeedMultiplier(float NewMultiplier);

	// Removes any multipliers that have modified enemy speed 
	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void RevertMovementSpeedMultiplier();

public:	
	void Init(FRogueEnemyInitializationArgs InitArgs);

	// Returns the raw multiplier value
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Enemy|Movement")
	float GetMovementSpeedMultiplier();

	// Blueprint Events

	// Notify the blueprint so it can activate its attack hitboxes
	// This has to be handled by each blueprint because each enemy has a different
	// hitbox layout
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Enemy|Combat")
	void OnActivateAttackHitboxes();

	// Notify the blueprint so it can deactivate its attack hitboxes
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Enemy|Combat")
	void OnDeactivateAttackHitboxes();

	// Notify the blueprint so it can setup it's patrol sequence
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Patrol")
	void OnSetupPatrol();

	// Notify the blueprint that the player has entered its patrol trigger volume
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Patrol")
	void OnPlayerEnteredPatrolTriggerVolume();

	// Notify the blueprint that the player has exited its patrol trigger volume
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Patrol")
	void OnPlayerExitedPatrolTriggerVolume();

	// Notify the blueprint that the player has entered its attack trigger volume
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Combat")
	void OnPlayerEnteredAttackTriggerVolume();

	// Notify the blueprint that the player has exited its attack trigger volume
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Combat")
	void OnPlayerExitedAttackTriggerVolume();

	// Notify the blueprint that the enemy has been stunned
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Combat")
	void OnHitStun();

	// Notify the blueprint that the stun has elapsed
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Combat")
	void OnStopHitStun();


	// Overlap event callbacks
	UFUNCTION()
	void OnBeginPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnBeginAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	// Called when the hit stun timer has completed
	void StopHitStun();

	// Stores the working index of current patrol point target
	int CurrentPatrolPointIndex = -1;

	// Stores our current speed multiplier
	float SpeedMultiplier = 1.0f;

	// Store the initial max walk speed set by the Character Movement Component in Blueprint 
	float InitialMaxWalkSpeed;

	// Handle for efficient management of the hit stun timer.  
	FTimerHandle TimerHandle_HitStun;

public:
 	ARogueEnemyAIControllerBase* GetEnemyController() const; 
};
