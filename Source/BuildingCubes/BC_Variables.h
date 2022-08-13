﻿#pragma once

#include "CoreMinimal.h"
#include "BC_Variables.generated.h"

UENUM()
enum class EActionType : uint8
{
	None,
	Building,
	Destroy
};

USTRUCT(BlueprintType)
struct FBlockMaterialPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UMaterialInterface* Preview;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UMaterialInterface* Base;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FLinearColor EmitColor;
};
