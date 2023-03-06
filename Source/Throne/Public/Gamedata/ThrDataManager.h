#pragma once

#include "ThrCharacterData.h"
#include "ThrTeamData.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ThrDataManager.generated.h"

#define GAMEDATA UThrDataManager::Ptr

UCLASS()
class THRONE_API UThrDataManager : public UObject
{
	GENERATED_BODY()
	
public:
	static UThrDataManager* Ptr;

	UThrDataManager();

	const TMap<FName, FThrTeamData> GetAllTeamsData() const;
	const FThrTeamData GetTeamData(const FName InTeamName) const;

	const FThrCharacterData GetCharacterData(const FName InCharacterName) const;
	const FThrCharacterAnimData GetCharacterAnimData(const FName InCharacterName) const;

private:
	UPROPERTY()
	const class UDataTable* TeamDataTable;

	UPROPERTY()
	const class UDataTable* CharacterDataTable;

	UPROPERTY()
	const class UDataTable* CharacterAnimDataTable;
};
