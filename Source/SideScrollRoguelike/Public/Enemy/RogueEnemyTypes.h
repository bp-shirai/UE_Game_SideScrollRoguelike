#pragma once

#include "RogueEnemyTypes.generated.h"

class USplineComponent;
class UBoxComponent;

/*
* This struct is used as a helper to both display the configurable UPROPERTY members
* in the details panel of an authorable actor (In this case, the enemy patrol rigs)
* and also allow conveniently passing all the necessary args in to an enemy at spawn.
*/
USTRUCT(BlueprintType)
struct SIDESCROLLROGUELIKE_API FRogueEnemyInitializationArgs
{
	GENERATED_BODY();

public:
	// The spline that this enemy will use 
	TObjectPtr<USplineComponent> PatrolSpline;

	// The volume that triggers enemy patrol behavior 
	TObjectPtr<UBoxComponent> PatrolTriggerBox;

	// The volume that triggers enemy attack behavior 
	TObjectPtr<UBoxComponent> AttackTriggerBox;

	UPROPERTY(EditInstanceOnly)
	bool LockXTransform = false;

	UPROPERTY(EditInstanceOnly)
	bool LockYTransform = false;

	UPROPERTY(EditInstanceOnly)
	bool LockZTransform = false;
};