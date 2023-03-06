#include "UI/Arena/ThrScoreBoard.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UThrScoreBoard::NativeConstruct()
{
	Super::NativeConstruct();

	AThrPlayerController::OnClientSetMatchState.AddUObject(this, &UThrScoreBoard::SetMatchState);
	AThrPlayerController::OnClientSetEnemyTeamName.AddUObject(this, &UThrScoreBoard::SetEnemyTeamData);
	
	FName TeamName;
	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		bLeftSide = GameInstance->IsLeftSide();
		TeamName = GameInstance->GetTeamName();
	}

	if (ProgressText)
		ProgressText->SetText(FText::FromString(TEXT("Waiting")));

	if (GAMEDATA)
	{
		const auto& TeamData = GAMEDATA->GetTeamData(TeamName);

		if (auto* TeamNameText = (bLeftSide ? LeftTeamNameText : RightTeamNameText))
		{
			TeamNameText->SetText(FText::FromName(TeamName));
			TeamNameText->SetColorAndOpacity(TeamData.SubColor);
		}

		if (auto* ScoreText = (bLeftSide ? LeftScoreText : RightScoreText))
		{
			ScoreText->SetVisibility(ESlateVisibility::Collapsed);
			ScoreText->SetColorAndOpacity(TeamData.MainColor);
		}

		if (auto* TeamEmblemImage = (bLeftSide ? LeftTeamEmblemImage : RightTeamEmblemImage))
			TeamEmblemImage->SetBrushFromTexture(TeamData.EmblemImage);

		if (auto* TeamEmblemShadeImage = (bLeftSide ? LeftTeamEmblemShadeImage : RightTeamEmblemShadeImage))
			TeamEmblemShadeImage->SetColorAndOpacity(TeamData.MainColor);

		if (auto* TeamEmblemStrokeImage = (bLeftSide ? LeftTeamEmblemStrokeImage : RightTeamEmblemStrokeImage))
			TeamEmblemStrokeImage->SetColorAndOpacity(TeamData.MainColor);

		if (auto* BackgroundImage = (bLeftSide ? LeftBackgroundImage : RightBackgroundImage))
			BackgroundImage->SetColorAndOpacity(TeamData.MainColor);
	}

	bLeftSide ? PlayAnimInitLeft() : PlayAnimInitRight();
}

void UThrScoreBoard::NativeTick(const FGeometry& InGeometry, float InDeltaTime)
{
	Super::NativeTick(InGeometry, InDeltaTime);

	if (ProgressText && MatchStateUpdateTime > FDateTime::MinValue())
	{
		if (MatchStateUpdateTime > FDateTime::UtcNow())
		{
			int32 RemainSeconds = FTimespan(MatchStateUpdateTime - FDateTime::UtcNow()).GetSeconds();
			ProgressText->SetText(FText::FromString(FString::FromInt(RemainSeconds)));
		}
		else
		{
			MatchStateUpdateTime = FDateTime::MinValue();
			ProgressText->SetText(FText::FromString(TEXT("Ready")));
		}
	}
}

void UThrScoreBoard::SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime)
{
	if (InMatchState == ThrMatchState::InProgress)
	{
		MatchStateUpdateTime = InMatchStateUpdateTime;

		if (!bInit)
		{
			bInit = true;
			bLeftSide ? PlayAnimInProgressLeft() : PlayAnimInProgressRight();
		}
	}
	else if (InMatchState == ThrMatchState::PrepareBattle)
	{
		MatchStateUpdateTime = FDateTime::MinValue();

		if (ProgressText)
			ProgressText->SetText(FText::FromString(TEXT("Ready")));
	}
	else if (InMatchState == ThrMatchState::StartBattle)
	{
		if (ProgressText)
			ProgressText->SetText(FText::FromString(TEXT("")));
	}
	else if (InMatchState == ThrMatchState::WaitingToPostMatch)
	{
		if (auto* GameState = Cast<AThrGameState>(UGameplayStatics::GetGameState(this)))
		{
			if (LeftScoreText)
				LeftScoreText->SetText(FText::FromString(FString::FromInt(GameState->LeftScore)));

			if (RightScoreText)
				RightScoreText->SetText(FText::FromString(FString::FromInt(GameState->RightScore)));
		}
	}
}

void UThrScoreBoard::SetEnemyTeamData(const FName InEnemyTeamName)
{
	if (!GAMEDATA)
		return;

	const auto& EnemyTeamData = GAMEDATA->GetTeamData(InEnemyTeamName);

	if (auto* EnemyTeamNameText = (bLeftSide ? RightTeamNameText : LeftTeamNameText))
	{
		EnemyTeamNameText->SetText(FText::FromName(InEnemyTeamName));
		EnemyTeamNameText->SetColorAndOpacity(EnemyTeamData.SubColor);
	}

	if (auto* EnemyScoreText = (bLeftSide ? RightScoreText : LeftScoreText))
	{
		EnemyScoreText->SetText(FText::FromString(TEXT("0")));
		EnemyScoreText->SetColorAndOpacity(EnemyTeamData.MainColor);
	}

	if (auto* EnemyTeamEmblemImage = (bLeftSide ? RightTeamEmblemImage : LeftTeamEmblemImage))
		EnemyTeamEmblemImage->SetBrushFromTexture(EnemyTeamData.EmblemImage);

	if (auto* EnemyTeamEmblemShadeImage = (bLeftSide ? RightTeamEmblemShadeImage : LeftTeamEmblemShadeImage))
		EnemyTeamEmblemShadeImage->SetColorAndOpacity(EnemyTeamData.MainColor);

	if (auto* EnemyTeamEmblemStrokeImage = (bLeftSide ? RightTeamEmblemStrokeImage : LeftTeamEmblemStrokeImage))
		EnemyTeamEmblemStrokeImage->SetColorAndOpacity(EnemyTeamData.MainColor);

	if (auto* EnemyBackgroundImage = (bLeftSide ? RightBackgroundImage : LeftBackgroundImage))
		EnemyBackgroundImage->SetColorAndOpacity(EnemyTeamData.MainColor);
}
