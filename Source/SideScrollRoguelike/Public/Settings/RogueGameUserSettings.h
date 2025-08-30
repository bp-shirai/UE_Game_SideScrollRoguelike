// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "RogueGameUserSettings.generated.h"

/**
 * Stores user settings for a Rogue game client (for example graphics and sound settings), with the ability to save and load to and from a file.
 */
UCLASS(config=GameUserSettings, configdonotcheckdefaults)
class SIDESCROLLROGUELIKE_API URogueGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:

	// The configurable main volume of the game 
	UPROPERTY(Config)
	float MainVolume = 1.0f;

	// The configurable music volume of the game 
	UPROPERTY(Config)
	float MusicVolume = 0.5f; 

	// The configurable SFX volume of the game
	UPROPERTY(Config)
	float SFXVolume = 1.0f; 
};
