#include "Gamedata/ThrDataManager.h"

#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

UThrDataManager* UThrDataManager::Ptr;

UThrDataManager::UThrDataManager()
	: TeamDataTable(ConstructorHelpers::FObjectFinder<UDataTable>(TEXT("/Game/Gamedata/DT_Team")).Object)
	, CharacterDataTable(ConstructorHelpers::FObjectFinder<UDataTable>(TEXT("/Game/Gamedata/DT_Character")).Object)
	, CharacterAnimDataTable(ConstructorHelpers::FObjectFinder<UDataTable>(TEXT("/Game/Gamedata/DT_CharacterAnim")).Object)
{
	Ptr = this;
}

const TMap<FName, FThrTeamData> UThrDataManager::GetAllTeamsData() const
{
	if (TeamDataTable)
	{
		TMap<FName, FThrTeamData> TeamsData;
		const auto& TeamNames = TeamDataTable->GetRowNames();
		for (const auto TeamName : TeamNames)
		{
			if (auto* TeamData = TeamDataTable->FindRow<FThrTeamData>(TeamName, TEXT("")))
				TeamsData.FindOrAdd(TeamName) = *TeamData;
		}

		return TeamsData;
	}

	return TMap<FName, FThrTeamData>();
}

const FThrTeamData UThrDataManager::GetTeamData(const FName InTeamName) const
{
	if (auto* TeamData = TeamDataTable->FindRow<FThrTeamData>(InTeamName, TEXT("")))
		return *TeamData;

	return FThrTeamData();
}

const FThrCharacterData UThrDataManager::GetCharacterData(const FName InCharacterName) const
{
	if (CharacterDataTable)
	{
		if (auto* CharacterData = CharacterDataTable->FindRow<FThrCharacterData>(InCharacterName, TEXT("")))
			return *CharacterData;
	}

	return FThrCharacterData();
}

const FThrCharacterAnimData UThrDataManager::GetCharacterAnimData(const FName InCharacterName) const
{
	if (CharacterAnimDataTable)
	{
		if (auto* CharacterAnimData = CharacterAnimDataTable->FindRow<FThrCharacterAnimData>(InCharacterName, TEXT("")))
			return *CharacterAnimData;
	}

	return FThrCharacterAnimData();
}
