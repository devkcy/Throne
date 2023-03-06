#include "UI/Arena/ThrBattleBoard.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"
#include "Player/Character/ThrCharacter.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UThrBattleBoard::NativeConstruct()
{
	Super::NativeConstruct();

	AThrPlayerController::OnClientSetMatchState.AddUObject(this, &UThrBattleBoard::SetMatchState);
	AThrPlayerController::OnClientEndBattle.AddUObject(this, &UThrBattleBoard::EndBattle);
	AThrPlayerController::OnClientSetEnemyTeamName.AddUObject(this, &UThrBattleBoard::SetEnemyTeamData);

	AThrCharacter::OnUpdateSP.BindUObject(this, &UThrBattleBoard::UpdateSP);

	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		bLeftSide = GameInstance->IsLeftSide();
		TeamName = GameInstance->GetTeamName();
	}

	if (GAMEDATA)
	{
		const auto& TeamData = GAMEDATA->GetTeamData(TeamName);

		if (auto* SpBar = (bLeftSide ? LeftSpBar : RightSpBar))
			SpBar->SetFillColorAndOpacity(TeamData.MainColor);

		if (auto* RapierImage = (bLeftSide ? LeftRapierImage : RightRapierImage))
			RapierImage->SetColorAndOpacity(TeamData.MainColor);

		if (auto* PlayerImage = (bLeftSide ? LeftPlayerImage : RightPlayerImage))
			PlayerImage->SetColorAndOpacity(TeamData.SubColor);
	}

	RoundPointImages = { Round1PointImage, Round2PointImage, Round3PointImage };
}

void UThrBattleBoard::NativeTick(const FGeometry& InGeometry, float InDeltaTime)
{
	Super::NativeTick(InGeometry, InDeltaTime);

	if (LeftSpBar)
	{
		const float IncreaseSP = LeftCurSP + 10.f * InDeltaTime;
		LeftCurSP = FMath::Min(IncreaseSP, LeftMaxSP);

		LeftSpBar->SetPercent(LeftCurSP / 100.f);
	}

	if (RightSpBar)
	{
		const float IncreaseSP = RightCurSP + 10.f * InDeltaTime;
		RightCurSP = FMath::Min(IncreaseSP, RightMaxSP);

		RightSpBar->SetPercent(RightCurSP / 100.f);
	}
}

void UThrBattleBoard::UpdateSP(const bool bInLeftSide, const float InMaxSP, const float InCurSP)
{
	if (bInLeftSide)
	{
		LeftMaxSP = InMaxSP;
		LeftCurSP = InCurSP;
	}
	else
	{
		RightMaxSP = InMaxSP;
		RightCurSP = InCurSP;
	}
}

void UThrBattleBoard::SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime)
{
	if (InMatchState == ThrMatchState::PrepareBattle)
	{
		LeftMaxSP = LeftCurSP = 100;
		RightMaxSP = RightCurSP = 100;

		if (LeftSpBar)
			LeftSpBar->SetPercent(1.f);

		if (RightSpBar)
			RightSpBar->SetPercent(1.f);

		if (auto* GameState = Cast<AThrGameState>(UGameplayStatics::GetGameState(this)))
		{
			TotalScore = GameState->LeftScore + GameState->RightScore;

			if (RoundNumText)
				RoundNumText->SetText(FText::FromString(FString::FromInt(TotalScore + 1)));
		}
	}
}

void UThrBattleBoard::SetEnemyTeamData(const FName InEnemyTeamName)
{
	if (!GAMEDATA)
		return;

	EnemyTeamName = InEnemyTeamName;
	const auto& EnemyTeamData = GAMEDATA->GetTeamData(EnemyTeamName);

	if (auto* EnemySpBar = (bLeftSide ? RightSpBar : LeftSpBar))
		EnemySpBar->SetFillColorAndOpacity(EnemyTeamData.MainColor);

	if (auto* EnemyRapierImage = (bLeftSide ? RightRapierImage : LeftRapierImage))
		EnemyRapierImage->SetColorAndOpacity(EnemyTeamData.MainColor);

	if (auto* EnemyPlayerImage = (bLeftSide ? RightPlayerImage : LeftPlayerImage))
		EnemyPlayerImage->SetColorAndOpacity(EnemyTeamData.SubColor);
}

void UThrBattleBoard::EndBattle(const int32 InBattleResult)
{
	const FName VictoryTeamName = (InBattleResult > 0 ? TeamName : EnemyTeamName);
	const auto& VictoryTeamData = GAMEDATA->GetTeamData(VictoryTeamName);

	if (!RoundPointImages.IsValidIndex(TotalScore))
		return;

	if (auto* RoundPointImage = RoundPointImages[TotalScore])
	{
		RoundPointImage->SetBrushFromTexture(VictoryTeamData.EmblemImage);
		RoundPointImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
	}
}
