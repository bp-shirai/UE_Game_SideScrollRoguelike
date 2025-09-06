// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/RogueHUD.h"

#include "CommonActivatableWidget.h"
#include "UI/RogueGameLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueHUD)

void ARogueHUD::BeginPlay()
{
    ensureMsgf(IsValid(RootLayoutWidget), TEXT("Invalid root widget in ARogueHUD::BeginPlay - check your class setup in editor"));

    // Assign the widget's owner to the owner of the HUD
    RootLayoutWidget->SetOwningPlayer(GetOwningPlayerController());

    // Add the widget to the player's screen as the root widget
    RootLayoutWidget->AddToPlayerScreen();

    // Load and add the first widget on the specified layer
    // Subsequent screens can just be stacked upon the root per layer using PushWidgetToLayerStack
    TSubclassOf<UCommonActivatableWidget> HUDWidgetClass = DefaultWidgetClass.LoadSynchronous();
    DefaultWidget                                        = RootLayoutWidget->PushWidgetToLayerStack(DefaultLayerName, HUDWidgetClass);
}
