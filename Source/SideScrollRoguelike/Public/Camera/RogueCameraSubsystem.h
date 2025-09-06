// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/RogueCameraTypes.h"
#include "RogueCameraSubsystem.generated.h"


class ARogueWorldSettings; 
class ARogueCamera; 
class ARoguePlayerCharacter; 



// Log category for the Rogue Camera Subsystem 
DECLARE_LOG_CATEGORY_EXTERN(LogRogueCameraSubsystem, Log, All); 

/**
* 
* A subsystem that interacts with the game camera.
* Shares the lifetime of the current world. 
* 
*/
UCLASS()
class SIDESCROLLROGUELIKE_API URogueCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public: 

	// Invoked when Begin Play is called on the world
	void OnWorldBeginPlay(UWorld& InWorld) override;

	// Gets the world position of the camera component 
	UFUNCTION(BlueprintCallable, Category = "Rogue|Camera")
	FVector GetCameraWorldPosition();

	// Sets the camera's movement mode 
	UFUNCTION(BlueprintCallable, Category = "Rogue|Camera")
	void SetCameraMode(ECameraMode NewMode); 

	// Sets the fixed location that the camera should interpolate to 
	UFUNCTION(BlueprintCallable, Category = "Rogue|Camera")
	void SetCameraFixedPointTarget(const FVector& TargetPosition);

	// Returns true when the player character is the owning player of this camera system 
	UFUNCTION(BlueprintCallable, Category = "Rogue|Camera")
	bool IsPlayerCameraOwner(ARoguePlayerCharacter* TargetPlayer); 

protected:

	// Spawns the camera from the camera class and initializes blend with local player
	void SetupCamera(); 

	// The settings used by the current world
	TObjectPtr<ARogueWorldSettings> RogueWorldSettings; 

	// The camera actor class to spawn based on the world settings 
	TSubclassOf<ARogueCamera> CameraActorClass;

	// The instance of the camera actor 
	TObjectPtr<ARogueCamera> CameraActorInstance;

	// The pawn that is the focus of this camera 
	TObjectPtr<ARoguePlayerCharacter> CameraOwner; 
};
