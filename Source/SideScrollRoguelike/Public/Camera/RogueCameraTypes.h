#pragma  once

#include "RogueCameraTypes.generated.h"

// This enum tracks which behavior the camera should implement
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	None, 
	Follow, 
	Fixed
};
