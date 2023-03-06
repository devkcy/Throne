#include "Player/Character/ThrSkinnedDecalComponent.h"

#include "AnimationRuntime.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

void UThrSkinnedDecalComponent::BeginPlay()
{
	Super::BeginPlay();

	DataTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, MaxDecals * 5, 1, RTF_RGBA16f, FLinearColor::Black, false);
	Materials.Empty();

	if (auto* Owner = GetOwner())
	{
		TArray<USkeletalMeshComponent*> Meshes;
		Owner->GetComponents<USkeletalMeshComponent>(Meshes);

		for (auto* Mesh : Meshes)
		{
			if (!Mesh)
				continue;

			if (!Mesh->MasterPoseComponent.IsValid())
				ParentMesh = Mesh;

			SetupMeshMaterials(Mesh);
		}
	}
}

void UThrSkinnedDecalComponent::SetupMeshMaterials(USkeletalMeshComponent* InMesh)
{
	if (!InMesh)
		return;

	const auto& MeshMaterials = InMesh->GetMaterials();
	for (int32 Index = 0; Index < MeshMaterials.Num(); ++Index)
	{
		auto* Material = InMesh->GetMaterial(Index);
		auto* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);

		if (IsValid(Material) && !DynamicMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(Material, GetOuter());

			InMesh->SetMaterial(Index, DynamicMaterial);
		}

		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValueByInfo(FMaterialParameterInfo(TEXT("DecalMax"), GlobalParameter, LayerIndex), MaxDecals * 5);
			DynamicMaterial->SetTextureParameterValueByInfo(FMaterialParameterInfo(TEXT("DecalInfo"), GlobalParameter, LayerIndex), DataTarget);
			DynamicMaterial->SetScalarParameterValueByInfo(FMaterialParameterInfo(TEXT("DecalLast"), GlobalParameter, LayerIndex), DecalLocations.Num());
			
			Materials.Add(DynamicMaterial);
		}
	}
}

void UThrSkinnedDecalComponent::SpawnDecal(const FVector& InLocation, const FQuat& InRotation, const FName InBoneName, const float InSize)
{
	if (!ParentMesh || !ParentMesh->SkeletalMesh || !Materials.IsValidIndex(0))
		return;

	const int32 BoneIndex = ParentMesh->GetBoneIndex(InBoneName);
	const auto& RefSkeleton = ParentMesh->SkeletalMesh->GetRefSkeleton();
	
	const auto& ReferenceTransform = FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, BoneIndex);
	const auto& BoneWorldTransform = ParentMesh->GetSocketTransform(InBoneName, RTS_World);
	
	const auto& DecalLocation = ReferenceTransform.TransformPosition(BoneWorldTransform.InverseTransformPosition(InLocation));
	const auto& DecalRotation = ReferenceTransform.TransformRotation(BoneWorldTransform.InverseTransformRotation(InRotation));

	LastDecalIndex = (LastDecalIndex + 1) % MaxDecals;
	DecalLocations.Add(DecalLocation);

	for (auto* Material : Materials)
	{
		if (IsValid(Material))
			Material->SetScalarParameterValueByInfo(FMaterialParameterInfo(TEXT("DecalLast"), GlobalParameter, LayerIndex), DecalLocations.Num());
	}

	UCanvas* Canvas;
	FVector2D CanvasSize;
	FDrawToRenderTargetContext RenderTargetContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, DataTarget, Canvas, CanvasSize, RenderTargetContext);
	if (!IsValid(Canvas))
		return;

	const float DataLocation = LastDecalIndex * 5.f + 1.f;
	Canvas->K2_DrawLine(FVector2D(DataLocation, 0.f), FVector2D(DataLocation, 1.f), 1.f, FLinearColor(DecalLocation));

	FVector BasisX, BasisY, BasisZ;
	const auto& DecalMatrix = FTransform(DecalRotation).ToMatrixNoScale();
	DecalMatrix.GetUnitAxes(BasisX, BasisY, BasisZ);

	float AdditionalDataValue = 0.f;
	if (auto* World = GetWorld())
		AdditionalDataValue = (World->IsPreviewWorld() ? 0.f : World->GetTimeSeconds());

	Canvas->K2_DrawLine(FVector2D(DataLocation + 1.f, 0.f), FVector2D(DataLocation + 1.f, 1.f), 1.f, FLinearColor(BasisX));
	Canvas->K2_DrawLine(FVector2D(DataLocation + 2.f, 0.f), FVector2D(DataLocation + 2.f, 1.f), 1.f, FLinearColor(BasisY));
	Canvas->K2_DrawLine(FVector2D(DataLocation + 3.f, 0.f), FVector2D(DataLocation + 3.f, 1.f), 1.f, FLinearColor(BasisZ));
	Canvas->K2_DrawLine(FVector2D(DataLocation + 4.f, 0.f), FVector2D(DataLocation + 4.f, 1.f), 1.f, FLinearColor(InSize, 0.f, AdditionalDataValue));

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, RenderTargetContext);
}
