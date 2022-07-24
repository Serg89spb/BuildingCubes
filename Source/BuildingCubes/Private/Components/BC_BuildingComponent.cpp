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

		CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Base,true);
	}
	if (M_CurrentAction == EActionType::Destroy)
	{
		M_isStartDestroy = true;
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
	UE_LOG(LogBC_BuildingComponent, Display, TEXT("M_CurrentMatIndex %i"), M_CurrentMatIndex);
}

void UBC_BuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	M_Owner = Cast<ABC_C_Character>(GetOwner());
	M_CurrentAction = EActionType::Building;
	M_CurrentMatIndex = 0;
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
			CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Preview,false);
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

	CreateAndSetMaterial(BlockMaterialPairs[M_CurrentMatIndex].Preview,true);
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

void UBC_BuildingComponent::CreateAndSetMaterial(UMaterialInterface* ParentMaterial, bool WithoutChangeIndex)
{
	if (M_DeltaIndex != M_CurrentMatIndex || WithoutChangeIndex)
	{
		if (IsValid(M_CurrentBlock) && IsValid(GetWorld()) && IsValid(ParentMaterial))
		{
			M_CurrentMat = UMaterialInstanceDynamic::Create(ParentMaterial, GetWorld());
			M_CurrentBlock->BC_MeshComponent->SetMaterial(0, M_CurrentMat);
			M_DeltaIndex = M_CurrentMatIndex;
			UE_LOG(LogBC_BuildingComponent, Display, TEXT("Change Mat"));
		}
	}
}
