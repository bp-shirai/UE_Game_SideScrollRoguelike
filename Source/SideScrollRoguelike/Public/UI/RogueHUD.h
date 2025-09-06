// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameplayTagContainer.h"
#include "RogueHUD.generated.h"

class UCommonActivatableWidget;
class URogueGameLayout;

/**
 * The HUD class is the base actor class of the heads-up display. It has a canvas and a debug canvas on which primitives can be drawn.
 * A HUD is also associated with a local player and is created as part of the game mode. 
 * In Rogue, we use this class as a wrapper to our root layout widget. 
 */
UCLASS(Abstract)
class SIDESCROLLROGUELIKE_API ARogueHUD : public AHUD
{
	GENERATED_BODY()

public: 
	// Called when play begins for this actor 
	virtual void BeginPlay() override; 

	// Getter for the root layout widget 
	TObjectPtr<URogueGameLayout> GetRootLayoutWidget() { return RootLayoutWidget; }

protected:
	// Layout class to use to setup the UI layout
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Rogue|UI")
	TObjectPtr<URogueGameLayout> RootLayoutWidget;

	// HUD widget class to create during startup
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Rogue|UI")
	TSoftClassPtr<UCommonActivatableWidget> DefaultWidgetClass;

	// The UI layer to create the HUD in
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Rogue|UI", meta = (Categories = "UI.Layer"))
	FGameplayTag DefaultLayerName;

	// The instanced default widget 
	TObjectPtr<UCommonActivatableWidget> DefaultWidget; 
};
