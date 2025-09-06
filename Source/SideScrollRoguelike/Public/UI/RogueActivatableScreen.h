// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RogueActivatableScreen.generated.h"

struct FInputActionValue;
class UInputAction;

/**
 * Common base widget for all activatable screen widgets to extend from.
 * Provides utility functions such as CloseScreen and "Back" Input Event handling.
 */

UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class SIDESCROLLROGUELIKE_API URogueActivatableScreen : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // Default construct for screens
    URogueActivatableScreen();

    // Removes this screen widget from viewport by calling into RogueGameLayout to remove it from the "MenuStack" layer.
    UFUNCTION(BlueprintCallable, Category = "Rogue|UI")
    void CloseScreen();

protected:
    // Define a custom event for blueprints to react to when the "back" action is invoked.
    // This lets the base class handle the CommonUI back action handle while the derived blueprints can implement behavior for this event
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Rogue|UI")
    void OnGenericBackTriggered();

    // This function allows us to refresh the CommonUI-commanded input config (e.g. Game, Menu)
    // Useful in some scenarios such as transitioning levels when the PlayerController is cycled out
    UFUNCTION(BlueprintCallable, Category = "Rogue|UI")
    void RefreshUIInputConfig();

    // Overrides CommonActivatableWidget's back action handler so we can invoke our back event and handle this per screen
    virtual bool NativeOnHandleBackAction() override;
};
