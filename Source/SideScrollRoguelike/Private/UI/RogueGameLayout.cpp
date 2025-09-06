// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/RogueGameLayout.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameLayout)

void URogueGameLayout::RegisterLayer(FGameplayTag LayerName, UCommonActivatableWidgetContainerBase* LayerWidget)
{
    if (!IsDesignTime())
    {
        Layers.Add(LayerName, LayerWidget);
    }
}

UCommonActivatableWidgetContainerBase* URogueGameLayout::GetLayerWidget(FGameplayTag LayerName) const
{
    return Layers.FindRef(LayerName);
}

void URogueGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
    for (const auto& LayerPair : Layers)
    {
        if (!LayerPair.Value->GetWidgetList().Contains(ActivatableWidget))
        {
            continue;
        }

        LayerPair.Value->RemoveWidget(*ActivatableWidget);
        break;
    }
}