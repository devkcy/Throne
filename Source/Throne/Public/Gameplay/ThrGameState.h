#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ThrGameState.generated.h"

namespace ThrMatchState
{
	const FName WaitingToStart = TEXT("WaitingToStart");
	const FName InProgress = TEXT("InProgress");
	const FName PrepareBattle = TEXT("PrepareBattle");
	const FName StartBattle = TEXT("StartBattle");
	const FName StopBattle = TEXT("StopBattle");
	const FName EndBattle = TEXT("EndBattle");
	const FName WaitingToPostMatch = TEXT("WaitingToPostMatch");
	const FName Surrender = TEXT("Surrender");
	const FName LeavingMap = TEXT("LeavingMap");
}

UCLASS()
class THRONE_API AThrGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 LeftScore;

	UPROPERTY(Replicated)
	int32 RightScore;
};
