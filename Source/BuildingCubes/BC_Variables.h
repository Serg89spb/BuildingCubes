#pragma once

#include "CoreMinimal.h"
#include "BC_Variables.generated.h"

UENUM()
enum class EActionType : uint8
{
	None,
	Building,
	Destroy
};
