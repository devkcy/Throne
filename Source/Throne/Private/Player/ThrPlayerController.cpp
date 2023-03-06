#include "Player/ThrPlayerController.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrCameraManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameMode.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerState.h"
#include "Player/Character/ThrCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

FOnTryToInProgress AThrPlayerController::OnTryToInProgress;
FOnTryToSetBattleState AThrPlayerController::OnTryToSetBattleState;

FOnClientNotifyArenaMessage AThrPlayerController::OnClientNotifyArenaMessage;
FOnClientSetMatchState AThrPlayerController::OnClientSetMatchState;
FOnClientSetEnemyTeamName AThrPlayerController::OnClientSetEnemyTeamName;
FOnClientSetEnemyCharacterName AThrPlayerController::OnClientSetEnemyCharacterName;
FOnClientEndBattle AThrPlayerController::OnClientEndBattle;
FOnInputSurrender AThrPlayerController::OnInputSurrender;

AThrPlayerController::AThrPlayerController()
{
	bShowMouseCursor = true;
	bAutoManageActiveCameraTarget = false;
}

void AThrPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	OnPossessedPawnChanged.AddDynamic(this, &AThrPlayerController::ChangePossessedPawn);

	AThrGameMode::OnServerNotifyArenaMessage.AddUObject(this, &AThrPlayerController::ClientNotifyMessage);
	AThrGameMode::OnServerSetMatchState.AddUObject(this, &AThrPlayerController::CheckSetMatchState);
	AThrGameMode::OnServerSetEnemyTeamName.AddUObject(this, &AThrPlayerController::CheckEnemyTeamName);
	AThrGameMode::OnServerSetEnemyCharacterName.AddUObject(this, &AThrPlayerController::CheckEnemyCharacterName);
}

void AThrPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		EnhancedInputComponent->BindAction(SurrenderInput, ETriggerEvent::Completed, this, &AThrPlayerController::InputSurrender);
}

void AThrPlayerController::PlayerTick(float InDeltaTime)
{
	Super::PlayerTick(InDeltaTime);

	if (CAMERA)
	{
		FRotator Rotation = CAMERA->GetActorRotation();
		Rotation.Yaw += (bLeftSide ? 90.f : -90.f);

		ClientSetRotation(Rotation);
	}
}

void AThrPlayerController::ClientInit_Implementation()
{
	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		InputSubsystem->AddMappingContext(InputMappingContext, 0);

	if (auto* World = GetWorld())
	{
		CAMERA = World->SpawnActor<AThrCameraManager>();
		SetViewTarget(CAMERA);
	}

	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		bLeftSide = GameInstance->IsLeftSide();
		ServerInit(bLeftSide, GameInstance->GetTeamName());
	}
}

bool AThrPlayerController::ServerTryToSetBattleState_Validate(const FName InBattleState)
{
	RPC_VALIDATE(InBattleState.IsValid());
	return true;
}

void AThrPlayerController::ServerTryToSetBattleState_Implementation(const FName InBattleState)
{
	if (InBattleState == ThrMatchState::WaitingToPostMatch)
	{
		UnPossess();

		if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
			OwningPlayerState->CharacterName = TEXT("");
	}

	OnTryToSetBattleState.ExecuteIfBound(InBattleState);
}

void AThrPlayerController::ChangePossessedPawn(APawn* InOldPawn, APawn* InNewPawn)
{
	if (InOldPawn)
		InOldPawn->Destroy();
}

bool AThrPlayerController::ServerInit_Validate(const bool bInLeftSide, const FName InTeamName)
{
	RPC_VALIDATE(InTeamName.IsValid());
	return true;
}

void AThrPlayerController::ServerInit_Implementation(const bool bInLeftSide, const FName InTeamName)
{
	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
	{
		OwningPlayerState->bLeftSide = bLeftSide = bInLeftSide;
		OwningPlayerState->TeamName = InTeamName;
	}

	OnTryToInProgress.ExecuteIfBound(this);
}

void AThrPlayerController::ClientNotifyMessage_Implementation(const FString& InMessage) const
{
	OnClientNotifyArenaMessage.ExecuteIfBound(InMessage);
}

void AThrPlayerController::CheckSetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime)
{
	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
		ClientSetMatchState(InMatchState, InMatchStateUpdateTime, OwningPlayerState->BattleResult);
}

void AThrPlayerController::ClientSetMatchState_Implementation(const FName InMatchState, const FDateTime InMatchStateUpdateTime, const int32 InBattleResult)
{
	OnClientSetMatchState.Broadcast(InMatchState, InMatchStateUpdateTime);

	if (InMatchState == ThrMatchState::WaitingToStart)
	{
		DisableInput(this);
	}
	else if (InMatchState == ThrMatchState::InProgress)
	{
		EnableInput(this);
	}
	else if (InMatchState == ThrMatchState::StartBattle)
	{
		bShowMouseCursor = false;

		if (auto* ControlledCharacter = Cast<AThrCharacter>(GetCharacter()))
			ControlledCharacter->SetInput(true);
	}
	else if (InMatchState == ThrMatchState::StopBattle)
	{
		bShowMouseCursor = true;

		if (auto* ControlledCharacter = Cast<AThrCharacter>(GetCharacter()))
			ControlledCharacter->SetInput(false);
	}
	else if (InMatchState == ThrMatchState::EndBattle)
	{
		OnClientEndBattle.Broadcast(InBattleResult);
	}
	else if (InMatchState == ThrMatchState::LeavingMap)
	{
		DisableInput(this);
	}
}

void AThrPlayerController::CheckEnemyTeamName(const FName InEnemyTeamName) const
{
	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
	{
		if (OwningPlayerState->TeamName != InEnemyTeamName)
			ClientSetEnemyTeamName(InEnemyTeamName);
	}
}

void AThrPlayerController::ClientSetEnemyTeamName_Implementation(const FName InEnemyTeamName) const
{
	OnClientSetEnemyTeamName.Broadcast(InEnemyTeamName);
}

void AThrPlayerController::CheckEnemyCharacterName(const FName InEnemyCharacterName) const
{
	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
	{
		if (OwningPlayerState->CharacterName != InEnemyCharacterName)
			ClientSetEnemyCharacterName(InEnemyCharacterName);
	}
}

void AThrPlayerController::ClientSetEnemyCharacterName_Implementation(const FName InEnemyCharacterName) const
{
	OnClientSetEnemyCharacterName.Broadcast(InEnemyCharacterName);
}

void AThrPlayerController::InputSurrender()
{
	if (auto* GameState = Cast<AThrGameState>(UGameplayStatics::GetGameState(this)))
	{
		const FName MatchState = GameState->GetMatchState();
		bool bAlwaysShowMouseCursor = (MatchState == TEXT("InProgress") || MatchState == TEXT("WaitingToPostMatch"));
		
		if (MatchState == TEXT("WaitingToStart") || MatchState == TEXT("LeavingMap"))
			return;

		bShowMouseCursor = (bAlwaysShowMouseCursor ? true : !bShowMouseCursor);
	}

	OnInputSurrender.ExecuteIfBound();
}
