// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/RogueCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Settings/RogueWorldSettings.h"
#include "Player/RoguePlayerCharacter.h"
#include "Player/RogueCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCamera)

ARogueCamera::ARogueCamera()
{
    PrimaryActorTick.bCanEverTick = true;

    // We want to update the camera's position based on the follow target
    // after physics has been applied. This means the follow target will be in its
    // new position for this frame.
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    DefaultSceneRoot           = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    SpringArmComponent         = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    CameraComponent            = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    BlockingMesh               = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockingMesh"));
    CameraMovementTriggerBox   = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraMovementTriggerBox"));

    // Attach the components in the desired hierarchy
    RootComponent = DefaultSceneRoot;
    SpringArmComponent->SetupAttachment(DefaultSceneRoot);
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraMovementTriggerBox->SetupAttachment(DefaultSceneRoot);
    BlockingMesh->SetupAttachment(DefaultSceneRoot);
}

void ARogueCamera::BeginPlay()
{
    FollowCharacterMovementComponent = FollowTarget->GetCharacterMovement<URogueCharacterMovementComponent>();
    ensureMsgf(FollowCharacterMovementComponent, TEXT("ARogueCamera::BeginPlay: Rogue Camera initialized with follow target but could not find URogueCharacterMovementComponent. Check your character setup."));
    ensureMsgf(RogueWorldSettings, TEXT("ARogueCamera::BeginPlay: Rogue Camera initialized invalid world settings. Check your world setup."));

    // Bind to the character's landed delagate
    FollowTarget->LandedDelegate.AddDynamic(this, &ThisClass::OnCharacterLanded);

    // Bind to the trigger box's overlap delegates
    CameraMovementTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::MovementTriggerBoxOverlapBegin);
    CameraMovementTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::MovementTriggerBoxOverlapEnd);

    // The camera's Y should only change when the camera movement changes
    CameraDefaultY = GetActorLocation().Y;

    Super::BeginPlay();
}

void ARogueCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unbind the trigger box's overlap delegates
    CameraMovementTriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::MovementTriggerBoxOverlapBegin);
    CameraMovementTriggerBox->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::MovementTriggerBoxOverlapEnd);

    Super::EndPlay(EndPlayReason);
}

void ARogueCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CameraMode)
    {
    case ECameraMode::Follow:
        TickFollowBehavior(DeltaTime);
        break;
    case ECameraMode::Fixed:
        TickFixedPoint(DeltaTime);
        break;
    case ECameraMode::None:
    default:
        break;
    }
}

void ARogueCamera::TickFollowBehavior(float DeltaTime)
{
    // If our follow target is no good, we should interpolate to the last known location
    if (!IsValid(FollowTarget) || !IsValid(FollowCharacterMovementComponent))
    {
        InterpolateToLastKnownFollowLocation(DeltaTime);
        return;
    }

    if (bIsOverlappingMovementBox)
    {
        CalculateLateralSpeed();
    }
    else
    {
        CurrentLateralSpeed = 0.0f;
    }

    InterpolateFromFollowTarget(DeltaTime);
}

void ARogueCamera::TickFixedPoint(float DeltaTime)
{
    if (bReachedFixedPoint)
    {
        return;
    }

    const FVector& CurrentLocation = GetActorLocation();
    const FVector NewLocation      = FMath::VInterpTo(CurrentLocation, FixedPointLocation, DeltaTime, FixedPointInterpolationSpeed);
    SetActorLocation(NewLocation);

    if (FVector::DistSquared(CurrentLocation, NewLocation) <= UE_KINDA_SMALL_NUMBER)
    {
        bReachedFixedPoint = true;
    }
}

void ARogueCamera::CalculateLateralSpeed()
{
    // Get the relevant locations that we need to calculate the speed
    const FVector& CurrentCameraLocation     = GetActorLocation();
    const FVector& CameraMovementBoxLocation = CameraMovementTriggerBox->GetComponentLocation();
    const FVector& BoxExtents                = CameraMovementTriggerBox->GetScaledBoxExtent();

    // Calculate the distance from the follow target to the left-most edge of the box
    float BoxEdgeX             = CameraMovementBoxLocation.X - BoxExtents.X;
    float FollowTargetX        = FollowTarget->GetActorLocation().X;
    float TargetToEdgeDistance = FMath::Abs(FollowTargetX - BoxEdgeX);

    // When the distance is less than zero, we don't want to follow along the X-axis
    if (TargetToEdgeDistance < 0)
    {
        CurrentLateralSpeed = 0.0f;
        return;
    }

    // When the edge is crossed, we want to speed up the camera based on how far the player is into the box
    float DistanceMultiplier = (TargetToEdgeDistance / BoxExtents.X);
    // We also apply a design authorable multiple
    // Then we multiply by the character's max walk speed. We always want the camera moving relative to the player's top speed.
    CurrentLateralSpeed = DistanceMultiplier * LateralFollowSpeedMultiplier * FollowCharacterMovementComponent->MaxWalkSpeed;
}

void ARogueCamera::InterpolateFromFollowTarget(float DeltaTime)
{
    const FVector& CurrentCameraLocation = GetActorLocation();
    const FVector& FollowTargetLocation  = FollowTarget->GetActorLocation();

    // Interpolate towards the desired X at a constant rate
    float StepX = FMath::FInterpConstantTo(CurrentCameraLocation.X, FollowTargetLocation.X, DeltaTime, CurrentLateralSpeed);

    // This case occurs when moving from a fixed point camera mode to follow
    float StepY = CameraDefaultY;

    if (CurrentCameraLocation.Y != CameraDefaultY)
    {
        // Interpolate towards the desired Y at the same lateral speed
        StepY = FMath::FInterpConstantTo(CurrentCameraLocation.Y, CameraDefaultY, DeltaTime, CurrentLateralSpeed);
    }

    // Default step to the camera's Z
    float StepZ = CurrentCameraLocation.Z;

    // Check if we're in Z bounds
    if (FollowTargetLocation.Z <= RogueWorldSettings->CutoffLowerBoundZ)
    {
        // Smooth toward the lower bound if we've exceeded it
        FollowTargetZ = RogueWorldSettings->CutoffLowerBoundZ;
        StepZ         = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetZ, DeltaTime, FollowZSpeed);
    }
    else if (FollowTargetLocation.Z >= RogueWorldSettings->CutoffUpperBoundZ)
    {
        // Smooth toward the upper bound if we've exceeded it
        FollowTargetZ = RogueWorldSettings->CutoffUpperBoundZ;
        StepZ         = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetZ, DeltaTime, FollowZSpeed);
    }
    else
    {
        // When the character has fallen below the TargetZ, the Z location should update
        if (FollowTargetLocation.Z < FollowTargetZ)
        {
            FollowTargetZ = FollowTargetLocation.Z;
        }

        // Smooth towards the target Z, regardless of lateral speed
        StepZ = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetZ, DeltaTime, FollowZSpeed);
    }

    // Set our actor location based on the step
    SetActorLocation(FVector(StepX, StepY, StepZ));
}

void ARogueCamera::InterpolateToLastKnownFollowLocation(float DeltaTime)
{
    const FVector& CurrentCameraLocation = GetActorLocation();
    float StepZ                          = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetZ, DeltaTime, FollowZSpeed);
    SetActorLocation(FVector(CurrentCameraLocation.X, CurrentCameraLocation.Y, StepZ));
}

void ARogueCamera::MovementTriggerBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CameraMode != ECameraMode::Follow)
    {
        return;
    }

    if (TObjectPtr<ARoguePlayerCharacter> Player = Cast<ARoguePlayerCharacter>(OtherActor))
    {
        if (Player != FollowTarget)
        {
            return;
        }

        bIsOverlappingMovementBox = true;
    }
}

void ARogueCamera::MovementTriggerBoxOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (TObjectPtr<ARoguePlayerCharacter> Player = Cast<ARoguePlayerCharacter>(OtherActor))
    {
        if (Player != FollowTarget)
        {
            return;
        }

        bIsOverlappingMovementBox = false;

        if (CameraMode != ECameraMode::Follow)
        {
            return;
        }

        // Edge case
        // If the player somehow manages to exit beyond the center of the movement box we should snap the camera into place where they've exited, immediately
        if (FollowTarget->GetActorLocation().X > CameraMovementTriggerBox->GetComponentLocation().X)
        {
            const FVector& CurrentTargetLocation = FollowTarget->GetActorLocation();
            const FVector& CurrentCameraLocation = GetActorLocation();
            SetActorLocation(FVector(CurrentTargetLocation.X, CurrentCameraLocation.Y, CurrentTargetLocation.Z));
        }
    }
}

void ARogueCamera::OnCharacterLanded(const FHitResult& Hit)
{
    const FVector& FollowTargetLocation = FollowTarget->GetActorLocation();
    FollowTargetZ                       = FollowTargetLocation.Z;
}

void ARogueCamera::SetCameraMode(ECameraMode NewMode)
{
    if (NewMode != CameraMode)
    {
        // Clean up state when exiting follow
        if (CameraMode == ECameraMode::Follow && bIsOverlappingMovementBox)
        {
            bIsOverlappingMovementBox = false;
        }

        if (NewMode == ECameraMode::Fixed)
        {
            bReachedFixedPoint = false;
        }
        else if (NewMode == ECameraMode::Follow)
        {
            // The player may have exited the fixed point camera beyond the follow movement box.
            // In this case, we should interpolate as if they are exceeding it.
            const FVector& BoxExtents           = CameraMovementTriggerBox->GetScaledBoxExtent();
            const float MovementTriggerBoxEdgeX = CameraMovementTriggerBox->GetComponentLocation().X - BoxExtents.X;
            if (FollowTarget->GetActorLocation().X >= MovementTriggerBoxEdgeX)
            {
                bIsOverlappingMovementBox = true;
            }
        }
    }

    CameraMode = NewMode;
}

FVector ARogueCamera::GetCameraComponentWorldPosition()
{
    return CameraComponent->GetComponentTransform().GetLocation();
}
