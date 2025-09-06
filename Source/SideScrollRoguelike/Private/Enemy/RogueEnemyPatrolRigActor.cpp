// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/RogueEnemyPatrolRigActor.h"

#include "Enemy/RogueEnemyPatrolRigComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueEnemyPatrolRigActor)

/*
 * All we need to do here is create our default subobject, the RogueEnemyPatrolRig, and set it as our root component.
 * This means that the hierarchy is generated for each instance as we place it into the level.
 */
ARogueEnemyPatrolRigActor::ARogueEnemyPatrolRigActor()
{
    PatrolRigComponent = CreateDefaultSubobject<URogueEnemyPatrolRigComponent>(TEXT("PatrolRig"));
    RootComponent      = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    PatrolRigComponent->SetupAttachment(RootComponent);
}
