// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingCubes/BC_Variables.h"
#include "Components/ActorComponent.h"
#include "BC_BuildingComponent.generated.h"


class ABC_C_BaseBlock;
class ABC_C_Character;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUILDINGCUBES_API UBC_BuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBC_BuildingComponent();

	void StartAction();
	void EndAction();
	void ChangeMaterial(float Value);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Subclass")
	TSubclassOf<AActor> BigBlockClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Trace")
	float MaxTraceDistance = 3000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Trace")
	float WithoutHitDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Materials")
	TArray<FBlockMaterialPair> BlockMaterialPairs;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool M_isStartBuilding;
	bool M_isStartDestroy;
	bool M_isStartPreview;

	int32 M_CurrentMatIndex;

	int32 M_DeltaIndex;

	EActionType M_CurrentAction;

	FVector M_BlocLoc;

	UPROPERTY()
	ABC_C_Character* M_Owner;

	UPROPERTY()
	ABC_C_BaseBlock* M_CurrentBlock;

	UPROPERTY()
	UMaterialInstanceDynamic* M_CurrentMat;

	void DrawTrace(TArray<AActor*> IgnoredActors, FHitResult& HitResult, float MaxDistance);
	bool CreateBlock(const FHitResult& HitResult);
	void CalculateStartEndLoc(float Distance, FVector& StartLoc, FVector& EndLoc);
	void SetBlockLocation(const FHitResult& HitResult);
	void CreateAndSetMaterial(UMaterialInterface* ParentMaterial, bool WithoutChangeIndex);
};
