// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BC_BuildingComponent.generated.h"


class ABC_C_Character;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUILDINGCUBES_API UBC_BuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBC_BuildingComponent();

	void StartAction();
	void EndAction();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ABC_C_Character* M_Owner;
};
