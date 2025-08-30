// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueMapDataAsset.generated.h"

class UObject;
class UWorld;

/**
 * Data asset classes store data related to a particular system. Primary data assets have asset bundle support which allows for manual loading/unloading from the Asset Manager. 
 * This is particularly useful for defining map data that we'd like to reference and then load. We use TSoftObjectPtr here so we can load the level asynchronously. 
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class URogueMapDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	// A soft object pointer the particular level we want to load
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Level;
};
