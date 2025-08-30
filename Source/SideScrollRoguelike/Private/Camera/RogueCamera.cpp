// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RogueCamera.h"

// Sets default values
ARogueCamera::ARogueCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARogueCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARogueCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

