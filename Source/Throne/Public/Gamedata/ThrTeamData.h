#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ThrTeamData.generated.h"

USTRUCT(BlueprintType)
struct FThrTeamData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FLinearColor MainColor;

	UPROPERTY(EditAnywhere)
	FLinearColor SubColor;

	UPROPERTY(EditAnywhere)
	class UTexture2D* EmblemImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* FlagImage;

	UPROPERTY(EditAnywhere)
	TArray<FName> CharacterNames;
};
