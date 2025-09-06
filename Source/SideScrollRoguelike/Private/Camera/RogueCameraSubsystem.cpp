// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RogueCameraSubsystem.h"

#include "Camera/RogueCamera.h"
#include "Settings/RogueWorldSettings.h"
#include "Player/RoguePlayerCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCameraSubsystem)

DEFINE_LOG_CATEGORY(LogRogueCameraSubsystem)

void URogueCameraSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	RogueWorldSettings = Cast<ARogueWorldSettings>(InWorld.GetWorldSettings()); 

	if (!RogueWorldSettings)
	{
		UE_LOG(LogRogueCameraSubsystem, Error, TEXT("URogueCameraSubsystem::OnWorldBeginPlay, could not get world settings - check your map settings %s"), *InWorld.GetName()); 
		return; 
	}

	CameraActorClass = RogueWorldSettings->CameraActorClass; 

	if (!CameraActorClass)
	{
		// This could be intentional and implies that this map should manage its own camera. i.e. The Main Menu map
		UE_LOG(LogRogueCameraSubsystem, Log, TEXT("URogueCameraSubsystem::OnWorldBeginPlay, no camera class found. Ignoring camera management in %s"), *InWorld.GetName()); 
		return; 
	}

	SetupCamera(); 
}

FVector URogueCameraSubsystem::GetCameraWorldPosition()
{
	return IsValid(CameraActorInstance) ? CameraActorInstance->GetCameraComponentWorldPosition() : FVector::ZeroVector;
}

void URogueCameraSubsystem::SetCameraMode(ECameraMode NewMode)
{
	CameraActorInstance->SetCameraMode(NewMode); 
}

void URogueCameraSubsystem::SetCameraFixedPointTarget(const FVector& TargetPosition)
{
	CameraActorInstance->SetCameraFixedPointTarget(TargetPosition); 
}

bool URogueCameraSubsystem::IsPlayerCameraOwner(ARoguePlayerCharacter* TargetPlayer)
{
	return TargetPlayer == CameraOwner;
}

void URogueCameraSubsystem::SetupCamera()
{
	// Get our pawn and pawn location from the local player controller 
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World->GetFirstLocalPlayerFromController()->GetPlayerController(World);
	APawn* Pawn = PlayerController->GetPawn();
	FVector PawnLocation = Pawn->GetActorLocation();

	// Defer spawn our camera, assign the follow target, then finish spawning 
	FTransform SpawnTransform = FTransform(PawnLocation);
	CameraActorInstance = World->SpawnActorDeferred<ARogueCamera>(CameraActorClass.Get(), SpawnTransform);

	CameraActorInstance->SetWorldSettings(RogueWorldSettings);
	CameraActorInstance->SetCameraMode(RogueWorldSettings->DefaultCameraMode); 

	// Always set the follow target based on the camera's owner 
	if (ARoguePlayerCharacter* TargetPlayer = Cast<ARoguePlayerCharacter>(Pawn))
	{
		CameraOwner = TargetPlayer; 
		CameraActorInstance->SetFollowTarget(TargetPlayer);
	}
	else
	{
		UE_LOG(LogRogueCameraSubsystem, Error, TEXT("URogueCameraSubsystem::SetupCamera, Could not cast pawn to Rogue Player Character. Check character setup."));
	}
	
	CameraActorInstance->FinishSpawning(SpawnTransform);

	// Blend the player controller's view target with the newly spawned camera 
	PlayerController->SetViewTargetWithBlend(CameraActorInstance);
}
