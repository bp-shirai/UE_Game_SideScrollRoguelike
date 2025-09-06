// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "Camera/RogueCameraTypes.h"
#include "RogueWorldSettings.generated.h"



/**
 * Actor containing all script accessible world properties. 
 * For Rogue, these are object references that each level will need as we load in.  
 */
UCLASS()
class SIDESCROLLROGUELIKE_API ARogueWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	// The default music for audio subsystem to play for a world once all loading has completed
	// (Can be 'None')
	UPROPERTY(EditDefaultsOnly, Category="Rogue|Audio|Sounds")
	TObjectPtr<USoundBase> WorldMusic; 

	// The default camera actor class that should spawn in the world as part of the camera subsystem 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category="Rogue|Camera")
	TSubclassOf<AActor> CameraActorClass; 

	// The default camera behavior tick mode that the Rogue camera should use 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category = "Rogue|Camera")
	ECameraMode DefaultCameraMode;

	// The lower bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="Rogue|Camera")
	float CutoffLowerBoundZ = -400.0f; 

	// The upper bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="Rogue|Camera")
	float CutoffUpperBoundZ = 750.0f; 	
};
