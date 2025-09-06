// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/RogueActivatableScreen.h"

#include "EnhancedInputComponent.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/UIActionBindingHandle.h"
#include "UI/RogueGameLayout.h"
#include "UI/RogueHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActivatableScreen)

URogueActivatableScreen::URogueActivatableScreen()
{
    // Ensure that all activatable screens can handle generic "back" actions by default
    bIsBackHandler = true;
}

void URogueActivatableScreen::RefreshUIInputConfig()
{
    if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this))
    {
        ActionRouter->RefreshUIInputConfig();
    }
}

bool URogueActivatableScreen::NativeOnHandleBackAction()
{
    // Invoke our back event
    OnGenericBackTriggered();

    return true;
}

void URogueActivatableScreen::CloseScreen()
{
    // Find the HUD from the owning player controller and remove this widget from its layer
    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        if (ARogueHUD* RogueHUD = PlayerController->GetHUD<ARogueHUD>())
        {
            if (URogueGameLayout* Root = RogueHUD->GetRootLayoutWidget())
            {
                Root->FindAndRemoveWidgetFromLayer(this);
            }
        }
    }
}