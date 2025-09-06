// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameplayTagContainer.h"
#include "RogueGameLayout.generated.h"

/**
 * This UI widget is the root widget we are using in our game.
 * It contains a list of UI layers (containers) where we can push our "activatable" widgets onto.
 * At any given time only one widget per layer is being displayed.
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class SIDESCROLLROGUELIKE_API URogueGameLayout : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // Create instance of target widget based on class and add to specified layer
    template <typename ActivatableWidgetT = UCommonActivatableWidget>
    ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
    {
        return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
    }

    // Create instance of target widget and calls the InitInstanceFunc after
    template <typename ActivatableWidgetT = UCommonActivatableWidget>
    ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
    {
        static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

        if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
        {
            return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
        }

        return nullptr;
    }

    // Get the layer widget for the given layer tag.
    UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName) const;

    // Find the widget if it exists on any of the layers and remove it from the layer.
    void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

protected:
    // Register a layer that widgets can be pushed onto.
    UFUNCTION(BlueprintCallable, Category = "Layer")
    void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

private:
    // The registered layers for the primary layout.
    UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
    TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> Layers;
};
