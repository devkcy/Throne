#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ThrCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FThrCharacterData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AThrCharacter> Class;

	UPROPERTY(EditAnywhere)
	class UTexture2D* ProfileImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* FullProfileImage;

	UPROPERTY(EditAnywhere)
	FString ContractWith;

	UPROPERTY(EditAnywhere)
	FString Details;
};

UENUM(BlueprintType)
enum EThrActionType
{
	NoneAction,

	Victory,
	Surrender,

	DeathHeadCenter,
	DeathHeadLeft,
	DeathHeadRight,
	DeathThorax,

	HitAbdomen,
	HitArmsLeft,
	HitArmsRight,
	HitLegsLeft,
	HitLegsRight,

	AttackTop,
	AttackTopLeft,
	AttackTopRight
};

USTRUCT(BlueprintType)
struct FThrCharacterAnimData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EThrActionType>, UAnimMontage*> AnimsByAction;
};
