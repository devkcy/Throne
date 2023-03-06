#include "UI/Arena/ThrArenaPanel.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"
#include "Player/ThrPlayerState.h"
#include "UI/ThrDialog.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"

void UThrArenaPanel::NativeConstruct()
{
	Super::NativeConstruct();

	AThrPlayerController::OnClientNotifyArenaMessage.BindUObject(this, &UThrArenaPanel::NotifyMessage);
	AThrPlayerController::OnClientSetMatchState.AddUObject(this, &UThrArenaPanel::SetMatchState);
	AThrPlayerController::OnClientEndBattle.AddUObject(this, &UThrArenaPanel::EndBattle);
	AThrPlayerController::OnClientSetEnemyTeamName.AddUObject(this, &UThrArenaPanel::SetEnemyTeamData);
	AThrPlayerController::OnInputSurrender.BindUObject(this, &UThrArenaPanel::InputSurrender);

	FName TeamName;
	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		bLeftSide = GameInstance->IsLeftSide();
		TeamName = GameInstance->GetTeamName();
	}

	auto* TeamFlagImage = (bLeftSide ? LeftTeamFlagImage : RightTeamFlagImage);
	if (TeamFlagImage && GAMEDATA)
	{
		const auto& TeamData = GAMEDATA->GetTeamData(TeamName);
		TeamFlagImage->SetBrushFromTexture(TeamData.FlagImage);
	}

	PlayAnimInit();
}

void UThrArenaPanel::ReplayAnimNotifyMessage()
{
	if (Messages.IsValidIndex(0))
		Messages.RemoveAt(0);

	if (NotifyText && Messages.IsValidIndex(0))
	{
		NotifyText->SetText(FText::FromString(Messages[0]));
		PlayAnimNotifyMessage();
	}
}

void UThrArenaPanel::TryToSetBattleState(const FName InBattleName)
{
	if (auto* Player = GetOwningPlayer<AThrPlayerController>())
		Player->ServerTryToSetBattleState(InBattleName);
}

void UThrArenaPanel::NotifyMessage(const FString& InMessage)
{
	if (!NotifyText || Messages.Add(InMessage))
		return;

	NotifyText->SetText(FText::FromString(InMessage));
	PlayAnimNotifyMessage();
}

void UThrArenaPanel::SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime)
{
	if (InMatchState == ThrMatchState::InProgress)
	{
		PlayAnimInProgress();
	}
	else if (InMatchState == ThrMatchState::PrepareBattle)
	{
		PlayAnimPrepareBattle();
	}
	else if (InMatchState == ThrMatchState::StartBattle)
	{
		if (MatchStateSwitcher)
			MatchStateSwitcher->SetActiveWidgetIndex(1);

		PlayAnimStartBattle();
	}
	else if (InMatchState == ThrMatchState::WaitingToPostMatch)
	{
		if (MatchStateSwitcher)
			MatchStateSwitcher->SetActiveWidgetIndex(0);
	}
	else if (InMatchState == ThrMatchState::Surrender)
	{
		if (MatchResultSwitcher)
		{
			if (auto* PlayerState = GetOwningPlayerState<AThrPlayerState>())
				MatchResultSwitcher->SetActiveWidgetIndex(PlayerState->bSurrender ? 1 : 0);
		}

		PlayAnimLeavingMap();
	}
	else if (InMatchState == ThrMatchState::LeavingMap)
	{
		if (MatchResultSwitcher)
		{
			if (auto* GameState = Cast<AThrGameState>(UGameplayStatics::GetGameState(this)))
			{
				const bool bWin = (bLeftSide ? GameState->LeftScore > GameState->RightScore : GameState->LeftScore < GameState->RightScore);
				MatchResultSwitcher->SetActiveWidgetIndex(bWin ? 0 : 1);
			}
		}

		PlayAnimLeavingMap();
	}
}

void UThrArenaPanel::SetEnemyTeamData(const FName InEnemyTeamName)
{
	auto* EnemyTeamFlagImage = (bLeftSide ? RightTeamFlagImage : LeftTeamFlagImage);
	if (EnemyTeamFlagImage && GAMEDATA)
	{
		const auto& EnemyTeamData = GAMEDATA->GetTeamData(InEnemyTeamName);
		EnemyTeamFlagImage->SetBrushFromTexture(EnemyTeamData.FlagImage);
	}
}

void UThrArenaPanel::EndBattle(const int32 InBattleResult)
{
	if (BattleResultSwitcher)
		BattleResultSwitcher->SetActiveWidgetIndex(InBattleResult + 1);

	PlayAnimEndBattle();
}

void UThrArenaPanel::InputSurrender()
{
	if (!Dialog)
		return;

	if (Dialog->IsVisible())
	{
		Dialog->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Are you sure to surrender?");
	DialogInfo.DialogType = Confirm;

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrArenaPanel::Surrender);
	Dialog->Show(DialogInfo);
}

void UThrArenaPanel::Surrender(const FString& InMessage)
{
	if (Dialog)
		Dialog->OnSubmitDialogInfo.Unbind();

	if (InMessage.IsEmpty())
		return;

	if (auto* PlayerState = GetOwningPlayerState<AThrPlayerState>())
	{
		PlayerState->bSurrender = true;
		PlayerState->ServerSetSurrender(true);
	}

	if (auto* Player = GetOwningPlayer<AThrPlayerController>())
		Player->ServerTryToSetBattleState(ThrMatchState::Surrender);
}
