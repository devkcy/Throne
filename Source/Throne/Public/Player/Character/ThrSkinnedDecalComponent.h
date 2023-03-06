#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThrSkinnedDecalComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THRONE_API UThrSkinnedDecalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void SpawnDecal(const FVector& InLocation, const FQuat& InRotation, const FName InBoneName, const float InSize);
	
private:
	void SetupMeshMaterials(USkeletalMeshComponent* InMesh);

	UPROPERTY()
	USkeletalMeshComponent* ParentMesh;
	
	UPROPERTY()
	class UTextureRenderTarget2D* DataTarget;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> Materials;
	TArray<FVector> DecalLocations;
	
	int32 LayerIndex = -1;
	int32 LastDecalIndex = -1;
	const int32 MaxDecals = 100;
};
