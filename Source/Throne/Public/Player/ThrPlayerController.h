#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ThrPlayerController.generated.h"

/* Send to Server */
DECLARE_DELEGATE_OneParam(FOnTryToInProgress, const TWeakObjectPtr<AThrPlayerController>&);
DECLARE_DELEGATE_OneParam(FOnTryToSetBattleState, const FName);

/* Send to UI */
DECLARE_DELEGATE_OneParam(FOnClientNotifyArenaMessage, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnClientSetMatchState, const FName, const FDateTime);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClientSetEnemyTeamName, const FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClientSetEnemyCharacterName, const FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClientEndBattle, const int32);
DECLARE_DELEGATE(FOnInputSurrender);

UCLASS()
class THRONE_API AThrPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	static FOnTryToInProgress OnTryToInProgress;
	static FOnTryToSetBattleState OnTryToSetBattleState;

	static FOnClientNotifyArenaMessage OnClientNotifyArenaMessage;
	static FOnClientSetMatchState OnClientSetMatchState;
	static FOnClientSetEnemyTeamName OnClientSetEnemyTeamName;
	static FOnClientSetEnemyCharacterName OnClientSetEnemyCharacterName;
	static FOnClientEndBattle OnClientEndBattle;
	static FOnInputSurrender OnInputSurrender;

	AThrPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void PlayerTick(float InDeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ClientInit();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryToSetBattleState(const FName InBattleState);
	
private:
	UFUNCTION()
	void ChangePossessedPawn(APawn* InOldPawn, APawn* InNewPawn);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInit(const bool bInLeftSide, const FName InTeam);
	
	UFUNCTION(Client, Reliable)
	void ClientNotifyMessage(const FString& InMessage) const;

	void CheckSetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime);

	UFUNCTION(Client, Reliable)
	void ClientSetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime, const int32 InBattleResult);
	
	void CheckEnemyTeamName(const FName InEnemyTeamName) const;

	UFUNCTION(Client, Reliable)
	void ClientSetEnemyTeamName(const FName InEnemyTeamName) const;

	void CheckEnemyCharacterName(const FName InEnemyCharacterName) const;

	UFUNCTION(Client, Reliable)
	void ClientSetEnemyCharacterName(const FName InEnemyCharacterName) const;

	void InputSurrender();

protected:
	UPROPERTY(EditDefaultsOnly)
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly)
	class UInputAction* SurrenderInput;

private:
	bool bLeftSide;
};
