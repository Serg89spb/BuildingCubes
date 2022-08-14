// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BC_BuildingComponent.h"

#include "Blocks/BC_C_BaseBlock.h"
#include "Camera/CameraComponent.h"
#include "Character/BC_C_Character.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogBC_BuildingComponent, All, All);

UBC_BuildingComponent::UBC_BuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBC_BuildingComponent::StartAction()
{
	if (M_CurrentAction == EActionType::Building)
	{
		M_isStartBuilding = true;
	}
	if (M_CurrentAction == EActionType::Destroy)
	{
		M_isStartDestroy = true;
	}
	UE_LOG(LogBC_BuildingComponent, Display, TEXT("Start Action"));
}

void UBC_BuildingComponent::EndAction()
{
	if (M_CurrentAction == EActionType::Building)
	{
		M_isStartBuilding = false;
		M_isStartPreview = false;

		CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Base);
		if(IsValid(M_CurrentMat) && IsValid(M_CurrentBlock))
		{
			M_CurrentMat->SetVectorParameterValue(FName("Emit Color"),BlockMaterialPairs[M_CurrentMatIndex].EmitColor);
			M_CurrentBlock->OnEndBuilding();
		}
	}
	if (M_CurrentAction == EActionType::Destroy)
	{
		M_isStartDestroy = false;
		Cast<ABC_C_BaseBlock>(M_HitActor)->OnResetDestroy();
	}
	UE_LOG(LogBC_BuildingComponent, Display, TEXT("End Action"));
}

void UBC_BuildingComponent::ChangeMaterial(float Value)
{
	M_CurrentMatIndex += FMath::Clamp(Value, -1.0f, 1.0f);

	if (M_CurrentMatIndex == BlockMaterialPairs.Num())
	{
		M_CurrentMatIndex = 0;
	}
	else if (M_CurrentMatIndex == -1)
	{
		M_CurrentMatIndex = BlockMaterialPairs.Num() - 1;
	}
	if (M_isStartPreview)
	{
		CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Preview);
	}
	UE_LOG(LogBC_BuildingComponent, Display, TEXT("M_CurrentMatIndex %i"), M_CurrentMatIndex);
}

void UBC_BuildingComponent::SwitchAction()
{
	if (M_CurrentAction == EActionType::Destroy || M_CurrentAction == EActionType::None)
	{
		M_CurrentAction = EActionType::Building;
		M_LightSphereMat->SetVectorParameterValue(FName("Color"),FLinearColor::Green);
	}
	else
	{
		M_CurrentAction = EActionType::Destroy;
		M_LightSphereMat->SetVectorParameterValue(FName("Color"),FLinearColor::Red);
	}
}

void UBC_BuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	M_Owner = Cast<ABC_C_Character>(GetOwner());
	M_CurrentAction = EActionType::Building;
	M_CurrentMatIndex = 0;

	if(IsValid(M_Owner) &&
		IsValid(M_Owner->BC_LightSphere) &&
		IsValid(M_Owner->BC_LightSphere->GetMaterial(0)) &&
		IsValid(GetWorld()))
	{
		M_LightSphereMat = UMaterialInstanceDynamic::Create(M_Owner->BC_LightSphere->GetMaterial(0),GetWorld());
		if(M_LightSphereMat)
		{
			M_Owner->BC_LightSphere->SetMaterial(0,M_LightSphereMat);
		}
	}
}


void UBC_BuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (M_isStartBuilding)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(M_Owner);
		IgnoredActors.AddUnique(M_CurrentBlock);

		FHitResult HitResult;
		DrawTrace(IgnoredActors, HitResult, MaxTraceDistance);
		if (CreateBlock(HitResult))
		{
			SetBlockLocation(HitResult);
		}
	}
	else if(M_isStartDestroy)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(M_Owner);

		FHitResult HitResult;
		DrawTrace(IgnoredActors, HitResult, MaxTraceDistance);
		if(HitResult.bBlockingHit && IsValid(HitResult.GetActor()))
		{
			if(HitResult.GetActor()->IsA<ABC_C_BaseBlock>())
			{
				Cast<ABC_C_BaseBlock>(HitResult.GetActor())->OnStartDestroy();
			}

			if(IsValid(M_HitActor) && M_HitActor->GetName() != HitResult.GetActor()->GetName())
			{
				Cast<ABC_C_BaseBlock>(M_HitActor)->OnResetDestroy();
			}
			M_HitActor = HitResult.GetActor();
		}
	}
}

void UBC_BuildingComponent::DrawTrace(TArray<AActor*> IgnoredActors, FHitResult& HitResult, float MaxDistance)
{
	if (!IsValid(M_Owner)) return;
	FVector StartLoc(ForceInitToZero), EndLoc(ForceInitToZero);
	CalculateStartEndLoc(MaxDistance, StartLoc, EndLoc);

	UKismetSystemLibrary::LineTraceSingle(GetWorld(),
	                                      StartLoc,
	                                      EndLoc,
	                                      TraceTypeQuery1,
	                                      false,
	                                      IgnoredActors,
	                                      EDrawDebugTrace::None,
	                                      HitResult,
	                                      true,
	                                      FLinearColor::Red,
	                                      FLinearColor::Green,
	                                      0.5f);
}

bool UBC_BuildingComponent::CreateBlock(const FHitResult& HitResult)
{
	if (M_isStartPreview) return true;
	if (!IsValid(BigBlockClass)) return false;
	FTransform Transform;
	Transform.SetLocation(HitResult.Location);
	M_CurrentBlock = GetWorld()->SpawnActor<ABC_C_BaseBlock>(BigBlockClass, Transform);

	CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Preview);
	return M_isStartPreview = IsValid(M_CurrentBlock);
}

void UBC_BuildingComponent::CalculateStartEndLoc(float Distance, FVector& StartLoc, FVector& EndLoc)
{
	StartLoc = M_Owner->BC_LightSphere->GetComponentLocation();
	EndLoc = StartLoc + M_Owner->FindComponentByClass<UCameraComponent>()->GetForwardVector() * Distance;
}

void UBC_BuildingComponent::SetBlockLocation(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		M_BlocLoc = HitResult.Location.GridSnap(50.0f) + HitResult.Normal * 50.0f;
	}
	else
	{
		FVector StartLoc(ForceInitToZero), EndLoc(ForceInitToZero);
		CalculateStartEndLoc(WithoutHitDistance, StartLoc, EndLoc);
		M_BlocLoc = EndLoc.GridSnap(50.0f);
	}

	const TArray<AActor*> IgnoredActors = {M_Owner, M_CurrentBlock};
	TArray<FHitResult> BoxHits;

	UKismetSystemLibrary::BoxTraceMulti(GetWorld(),
	                                    M_BlocLoc,
	                                    M_BlocLoc,
	                                    FVector(50.0f),
	                                    FRotator::ZeroRotator,
	                                    TraceTypeQuery1,
	                                    false,
	                                    IgnoredActors,
	                                    EDrawDebugTrace::None,
	                                    BoxHits,
	                                    true);

	for (const auto& OneHit : BoxHits)
	{
		M_BlocLoc += OneHit.Normal;
		//UE_LOG(LogBC_BuildingComponent, Display, TEXT("Hit: %s"), *OneHit.Normal.ToString());
	}

	M_CurrentBlock->SetActorLocation(M_BlocLoc);
}

void UBC_BuildingComponent::CreateAndSetMaterial(UMaterialInterface* ParentMaterial)
{
	if (IsValid(M_CurrentBlock) && IsValid(GetWorld()) && IsValid(ParentMaterial))
	{
		M_CurrentMat = UMaterialInstanceDynamic::Create(ParentMaterial, GetWorld());
		M_CurrentBlock->BC_MeshComponent->SetMaterial(0, M_CurrentMat);
		M_DeltaIndex = M_CurrentMatIndex;
		UE_LOG(LogBC_BuildingComponent, Display, TEXT("Change Mat"));
	}
}
