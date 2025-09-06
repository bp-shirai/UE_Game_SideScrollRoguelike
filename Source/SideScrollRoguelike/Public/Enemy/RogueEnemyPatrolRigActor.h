// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueEnemyPatrolRigActor.generated.h"

class URogueEnemyPatrolRigComponent;

/*
 * This is an example of an actor class with a default subobject generated at construction time. This is useful
 * for guaranteeing there is a required component placed into the hierarchy that cannot be removed.
 *
 * This essentially wraps our RogueEnemyPatrolRigComponent to allow it to be placed anywhere in
 * the level. This provides additional flexibility, as we can either place this actor into the level to
 * place a patrol at any arbitrary location, or attach the component directly to an object instance
 * that already exists in the level.
 */

UCLASS()
class SIDESCROLLROGUELIKE_API ARogueEnemyPatrolRigActor : public AActor
{
    GENERATED_BODY()

public:
    // Default constructor
    ARogueEnemyPatrolRigActor();

public:
    // The created patrol rig component that this actor owns
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<URogueEnemyPatrolRigComponent> PatrolRigComponent;
};
