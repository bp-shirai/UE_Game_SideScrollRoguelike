// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/RogueEnemyPatrolRigActor.h"

// Sets default values
ARogueEnemyPatrolRigActor::ARogueEnemyPatrolRigActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARogueEnemyPatrolRigActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARogueEnemyPatrolRigActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

