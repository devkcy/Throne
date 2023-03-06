#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ThrGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerNotifyArenaMessage, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnServerSetMatchState, const FName, const FDateTime);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerSetEnemyTeamName, const FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerSetEnemyCharacterName, const FName);

UCLASS()
class THRONE_API AThrGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	static FOnServerNotifyArenaMessage OnServerNotifyArenaMessage;
	static FOnServerSetMatchState OnServerSetMatchState;
	static FOnServerSetEnemyTeamName OnServerSetEnemyTeamName;
	static FOnServerSetEnemyCharacterName OnServerSetEnemyCharacterName;

	AThrGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void InitGame(const FString& InMapName, const FString& InOptions, FString& OutErrorMessage) override;
	virtual void PostLogin(APlayerController* InNewPlayer) override;
	virtual void Tick(float InDeltaSeconds) override;

protected:
	virtual void OnMatchStateSet() override;

private:
	void TryToInProgress(const TWeakObjectPtr<class AThrPlayerController>& InNewPlayer);
	void TryToSetBattleState(const FName InBattleState);
	void Surrender();

	const bool SpawnCharacter(const TWeakObjectPtr<class AThrPlayerController>& InPlayer);
	
	void EndBattle();

	FDateTime MatchStateUpdateTime;

	TArray<TWeakObjectPtr<class AThrPlayerController>> Players;
	int32 PreparedPlayersNum;
};
