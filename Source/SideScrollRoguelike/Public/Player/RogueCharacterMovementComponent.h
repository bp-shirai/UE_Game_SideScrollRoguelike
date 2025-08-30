// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "RogueCharacterMovementComponent.generated.h"

class ARoguePlayerCharacter;

//namespace EEasingFunc
//{
//	enum Type : int; 
//}

/**
 * 
 */
UCLASS()
class SIDESCROLLROGUELIKE_API URogueCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	URogueCharacterMovementComponent();

	//~ Begin UActorComponent Interface 
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction); 
	//~ End UActorComponent Interface

	//~ Begin UCharacterMovementComponent Interface

	// Returns true if current movement state allows an attempt at jumping. Used by Character::CanJump().
	virtual bool CanAttemptJump() const override; 

	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override; 

	// Called after MovementMode has changed. Base implementation does special handling for starting certain modes, then notifies the CharacterOwner.
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	// Called if bNotifyApex is true and character has just passed the apex of its jump. 
	virtual void NotifyJumpApex() override; 

	// Explicitly informs the character movement component that a jump has stopped 
	void StopJumpInput(); 

	// Performs a jump off an enemy
	void DoEnemyJump(); 

	public: 
	// The height of the character's jump apex. 
	// When the player holds down the button for the Character's JumpMaxHoldTime, this height will be reached 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", UIMin="1"), Category="Parrot Movement: Jumping / Falling")
	float ApexJumpHeight = 500.0f; 
	
	// The amount of time that the player has to input a jump input after walking off a ledge 
	UPROPERTY(EditAnywhere, BluePrintReadWrite, meta=(ClampMin="0", UIMin="0"), Category="Parrot Movement: Jumping / Falling")
	float CoyoteTime = 0.1f; 

	// A multiplier that increases gravity if the button is released early so the apex is reached more quickly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", UIMin="1"), Category="Parrot Movement: Jumping / Falling")
	float EarlyReleaseGravityMultiplier = 2.0f; 

	// The gravity scale we apply when the player reaches the apex. We ease from this to the max gravity based on the easing type.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", UIMin="1"), Category="Parrot Movement: Jumping / Falling")
	float FallBeginGravityScale = 2.5f; 

	// The maximum gravity scale we will reach after easing from the beginning fall gravity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", UIMin="1"), Category="Parrot Movement: Jumping / Falling")
	float FallMaxGravityScale = 5.0f; 

	// The time our blend will take to transition from the FallBeginGravityScale to FallMaxGravityScale.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", UIMin="0.1"), Category="Parrot Movement: Jumping / Falling")
	float FallGravityBlendTime = 0.25f; 

	// The easing function type that should be applied when interpolating gravity 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parrot Movement: Jumping / Falling")
	TEnumAsByte<EEasingFunc::Type> GravityEasingType; 

	// Controls the degree of the curve when using EaseIn, EaseOut, EaseInOut easing function types  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", EditConditionHides, EditCondition = "GravityEasingType == EEasingFunc::EaseIn||GravityEasingType == EEasingFunc::EaseOut||GravityEasingType == EEasingFunc::EaseInOut"), Category="Parrot Movement: Jumping / Falling")
	double FallGravityEaseBlend = 2.0; 

	// The step applied when using the Step easing function type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", EditConditionHides, EditCondition="GravityEasingType == EEasingFunc::Step"), Category = "Parrot Movement: Jumping / Falling")
	int FallGravityStep = 2;


	protected:

	// Ticks any movement related timers that are custom to the Parrot Character Movement Component 
	void TickMovementTimers(float DeltaTime); 

	// Interpolates the gravity when the character enter a falling state
	void InterpolateFallingGravity(float DeltaTime); 

protected: 

	// The default gravity scale that the character starts with 
	float DefaultGravityScale; 

	// Whether or not the character is in the air 
	bool bIsAirborne;

	// How long the character has been in the air 
	float AirborneTime; 

	// The amount of time that the character has been in a falling descent 
	float FallTime;

	// Whether the character's falling gravity is being applied to the character
	bool bApplyFallingGravity; 

	// Whether or not the input for a jump is actively being processed 
	bool bJumpInputActive; 

	// When true, gravity will not reset when a jump is performed
	bool bIgnoreInitialJumpStateReset; 

	// When true, allows the player to pass the 'Can Jump' for an enemy jump
	bool bPerformingEnemyJump; 
};
