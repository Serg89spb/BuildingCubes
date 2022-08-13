// Fill out your copyright notice in the Description page of Project Settings.


#include "Blocks/BC_C_BaseBlock.h"

ABC_C_BaseBlock::ABC_C_BaseBlock()
{
	PrimaryActorTick.bCanEverTick = true;

	BC_MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("BC_MeshComponent");
	SetRootComponent(BC_MeshComponent);
}

void ABC_C_BaseBlock::OnEndBuilding()
{
	M_StartFadeEmit = true;
	if(IsValid(BC_MeshComponent) && IsValid(BC_MeshComponent->GetMaterial(0)))
	{
		M_BlockMat = Cast<UMaterialInstanceDynamic>(BC_MeshComponent->GetMaterial(0));
	}
}

void ABC_C_BaseBlock::BeginPlay()
{
	Super::BeginPlay();
	M_FXPowerCounter = FadeTime;
}
void ABC_C_BaseBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(M_StartFadeEmit)
	{
		if(M_FXPowerCounter > 0)
		{
			M_FXPowerCounter -= DeltaTime;
			UE_LOG(LogTemp,Display,TEXT("M_FXPowerCounter: %f"),M_FXPowerCounter);
			const float Power = MaxFadePower * M_FXPowerCounter;
			ChangeEmitPower(Power);
		}
		else
		{
			ResetEmitToZero();
		}
	}
}

void ABC_C_BaseBlock::ChangeEmitPower(float Power)
{
	if(!IsValid(M_BlockMat)) return;
	M_BlockMat->SetScalarParameterValue(FName("Emit Power"),Power * Power);
}

void ABC_C_BaseBlock::ResetEmitToZero()
{
	if(!IsValid(M_BlockMat)) return;
	M_BlockMat->SetScalarParameterValue(FName("Emit Power"),0.0f);
	M_FXPowerCounter = 0.0f;
	M_StartFadeEmit = false;
	UE_LOG(LogTemp,Warning,TEXT("Finsih Fade"));
}

