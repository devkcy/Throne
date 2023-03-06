#include "Gameplay/ThrGameMode.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"
#include "Player/ThrPlayerState.h"
#include "Player/Character/ThrCharacter.h"

#include "Kismet/GameplayStatics.h"

FOnServerNotifyArenaMessage AThrGameMode::OnServerNotifyArenaMessage;
FOnServerSetMatchState AThrGameMode::OnServerSetMatchState;
FOnServerSetEnemyTeamName AThrGameMode::OnServerSetEnemyTeamName;
FOnServerSetEnemyCharacterName AThrGameMode::OnServerSetEnemyCharacterName;

AThrGameMode::AThrGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDelayedStart = true;
	PrimaryActorTick.TickInterval = 0.1f;
	MatchStateUpdateTime = FDateTime::MinValue();
}

void AThrGameMode::InitGame(const FString& InMapName, const FString& InOptions, FString& OutErrorMessage)
{
	Super::InitGame(InMapName, InOptions, OutErrorMessage);

	AThrPlayerController::OnTryToInProgress.BindUObject(this, &AThrGameMode::TryToInProgress);
	AThrPlayerController::OnTryToSetBattleState.BindUObject(this, &AThrGameMode::TryToSetBattleState);
}

void AThrGameMode::PostLogin(APlayerController* InNewPlayer)
{
	Super::PostLogin(InNewPlayer);

	if (auto* NewPlayer = Cast<AThrPlayerController>(InNewPlayer))
	{
		NewPlayer->InitPlayerState();
		NewPlayer->ClientInit();
	}
}

void AThrGameMode::Tick(float InDeltaSeconds)
{
	Super::Tick(InDeltaSeconds);
	
	if (MatchStateUpdateTime > FDateTime::MinValue() && MatchStateUpdateTime <= FDateTime::UtcNow())
	{
		MatchStateUpdateTime = FDateTime::MinValue();

		if (MatchState == ThrMatchState::WaitingToStart)
		{
			SetMatchState(ThrMatchState::InProgress);
		}
		else if (MatchState == ThrMatchState::StopBattle)
		{
			SetMatchState(ThrMatchState::EndBattle);
		}
		else if (MatchState == ThrMatchState::WaitingToPostMatch)
		{
			if (auto* OwningGameState = GetGameState<AThrGameState>())
			{
				const bool bEndMatch = (OwningGameState->LeftScore > 1 || OwningGameState->RightScore > 1);
				SetMatchState(bEndMatch ? ThrMatchState::LeavingMap : ThrMatchState::InProgress);
			}
		}
		else if (MatchState == ThrMatchState::Surrender || MatchState == ThrMatchState::LeavingMap)
		{
			UGameplayStatics::OpenLevel(this, TEXT("Lobby"));
		}
	}
}

void AThrGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	MatchStateUpdateTime = FDateTime::MinValue();

	if (MatchState == ThrMatchState::InProgress)
	{
		OnServerNotifyArenaMessage.Broadcast(TEXT("Choose your gladiator"));
		MatchStateUpdateTime = FDateTime::UtcNow() + FTimespan::FromSeconds(30.f);
	}
	else if (MatchState == ThrMatchState::PrepareBattle)
	{
		OnServerNotifyArenaMessage.Broadcast(TEXT("Prepare for battle"));
	}
	else if (MatchState == ThrMatchState::StartBattle)
	{
		OnServerNotifyArenaMessage.Broadcast(TEXT("Welcome to the Colosseum"));
	}
	else if (MatchState == ThrMatchState::StopBattle)
	{
		MatchStateUpdateTime = FDateTime::UtcNow() + FTimespan::FromSeconds(0.3f);
	}
	else if (MatchState == ThrMatchState::EndBattle)
	{
		EndBattle();
	}
	else if (MatchState == ThrMatchState::WaitingToPostMatch)
	{
		MatchStateUpdateTime = FDateTime::UtcNow() + FTimespan::FromSeconds(1.f);
	}
	else if (MatchState == ThrMatchState::Surrender || MatchState == ThrMatchState::LeavingMap)
	{
		MatchStateUpdateTime = FDateTime::UtcNow() + FTimespan::FromSeconds(2.f);
	}
	
	OnServerSetMatchState.Broadcast(MatchState, MatchStateUpdateTime);
}

void AThrGameMode::TryToInProgress(const TWeakObjectPtr<class AThrPlayerController>& InNewPlayer)
{
	if (Players.Add(InNewPlayer) < 1)
		return;

	for (auto& Player : Players)
	{
		if (!Player.IsValid())
			return;

		if (auto* PlayerState = Player->GetPlayerState<AThrPlayerState>())
			OnServerSetEnemyTeamName.Broadcast(PlayerState->TeamName);
	}

	MatchStateUpdateTime = FDateTime::UtcNow() + FTimespan::FromSeconds(1.f);
}

void AThrGameMode::TryToSetBattleState(const FName InBattleState)
{
	if (InBattleState == ThrMatchState::PrepareBattle)
	{
		for (auto& Player : Players)
		{
			if (!SpawnCharacter(Player))
				return;
		}
	}
	else if (InBattleState == ThrMatchState::StartBattle)
	{
		if (++PreparedPlayersNum < 2)
			return;
	}
	else if (InBattleState == ThrMatchState::WaitingToPostMatch)
	{
		if (--PreparedPlayersNum > 0)
			return;
	}

	SetMatchState(InBattleState);
}

const bool AThrGameMode::SpawnCharacter(const TWeakObjectPtr<AThrPlayerController>& InPlayer)
{
	if (!InPlayer.IsValid())
		return false;

	if (Cast<AThrCharacter>(InPlayer->GetPawn()))
		return true;

	FString Side;
	FName CharacterName;
	if (auto* PlayerState = InPlayer->GetPlayerState<AThrPlayerState>())
	{
		Side = (PlayerState->bLeftSide ? TEXT("Left") : TEXT("Right"));
		CharacterName = PlayerState->CharacterName;
	}

	auto* World = GetWorld();
	if (Side.IsEmpty() || CharacterName.IsNone() || !World)
		return false;

	if (auto* SpawnSpot = FindPlayerStart(InPlayer.Get(), Side))
	{
		const auto& CharacterClass = GAMEDATA->GetCharacterData(CharacterName).Class;
		const auto& SpawnTransform = SpawnSpot->GetTransform();

		if (auto* SpawnCharacter = World->SpawnActor<AThrCharacter>(CharacterClass, SpawnTransform, FActorSpawnParameters()))
		{
			InPlayer->Possess(SpawnCharacter);
			OnServerSetEnemyCharacterName.Broadcast(CharacterName);

			return true;
		}
	}

	return false;
}

void AThrGameMode::EndBattle()
{
	auto* OwningGameState = GetGameState<AThrGameState>();
	if (!OwningGameState)
		return;

	TArray<AThrPlayerState*> PlayerStates;
	PlayerStates.Init(nullptr, 2);

	for (const auto& Player : Players)
	{
		if (!Player.IsValid())
			return;
		
		if (auto* PlayerState = Player->GetPlayerState<AThrPlayerState>())
		{
			const int32 IndexBySide = (PlayerState->bLeftSide ? 0 : 1);
			PlayerStates[IndexBySide] = PlayerState;
		}
	}

	if (!PlayerStates[0] || !PlayerStates[1])
		return;

	if (PlayerStates[0]->bDead && PlayerStates[1]->bDead)
	{
		PlayerStates[0]->BattleResult = PlayerStates[1]->BattleResult = 0;
		return;
	}

	bool bLeftWin;
	if (PlayerStates[0]->bDead || PlayerStates[1]->bDead)
	{
		bLeftWin = PlayerStates[1]->bDead;
	}
	else if (PlayerStates[0]->MaxSP == PlayerStates[1]->MaxSP)
	{
		bLeftWin = (PlayerStates[0]->LastTakeDamageTime < PlayerStates[1]->LastTakeDamageTime);
	}
	else
	{
		bLeftWin = (PlayerStates[0]->MaxSP > PlayerStates[1]->MaxSP);
	}

	PlayerStates[0]->BattleResult = (bLeftWin ? 1 : -1);
	PlayerStates[1]->BattleResult = (bLeftWin ? -1 : 1);

	bLeftWin ? ++OwningGameState->LeftScore : ++OwningGameState->RightScore;
}
