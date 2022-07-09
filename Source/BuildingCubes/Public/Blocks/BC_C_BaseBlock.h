// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BC_C_BaseBlock.generated.h"

UCLASS()
class BUILDINGCUBES_API ABC_C_BaseBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	ABC_C_BaseBlock();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Components")
	UStaticMeshComponent* BC_MeshComponent;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
