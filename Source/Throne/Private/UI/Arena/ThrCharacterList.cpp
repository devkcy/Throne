#include "UI/Arena/ThrCharacterList.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"
#include "Player/ThrPlayerState.h"
#include "UI/Arena/ThrCharacterListItem.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"

void UThrCharacterList::NativeConstruct()
{
	Super::NativeConstruct();

	AThrPlayerController::OnClientSetMatchState.AddUObject(this, &UThrCharacterList::SetMatchState);
	AThrPlayerController::OnClientSetEnemyTeamName.AddUObject(this, &UThrCharacterList::SetEnemyTeamData);
	AThrPlayerController::OnClientSetEnemyCharacterName.AddUObject(this, &UThrCharacterList::SetEnemyCharacterData);

	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		bLeftSide = GameInstance->IsLeftSide();
		TeamName = GameInstance->GetTeamName();
	}

	if (LockInButton)
		LockInButton->OnClicked.AddDynamic(this, &UThrCharacterList::ClickLockInButton);

	if (GAMEDATA)
	{
		const auto& TeamData = GAMEDATA->GetTeamData(TeamName);

		if (CharacterList)
		{
			CharacterList->OnItemSelectionChanged().AddUObject(this, &UThrCharacterList::SelectCharacterListItem);

			for (const auto CharacterName : TeamData.CharacterNames)
			{
				if (auto* ListItemData = NewObject<UThrCharacterListItemData>())
				{
					ListItemData->CharacterName = CharacterName;

					CharacterList->AddItem(ListItemData);
				}
			}
		}

		if (auto* CharacterFullProfileBorder = (bLeftSide ? LeftCharacterFullProfileBorder : RightCharacterFullProfileBorder))
			CharacterFullProfileBorder->SetColorAndOpacity(TeamData.MainColor);
	}

	Init();
	bLeftSide ? PlayAnimInitLeft() : PlayAnimInitRight();
}

void UThrCharacterList::NativeTick(const FGeometry& InGeometry, float InDeltaTime)
{
	Super::NativeTick(InGeometry, InDeltaTime);

	if (MatchState == ThrMatchState::InProgress && MatchStateUpdateTime <= FDateTime::UtcNow())
		ClickLockInButton();
}

void UThrCharacterList::Init()
{
	bLockIn = false;

	if (LockInButton)
		LockInButton->SetIsEnabled(false);

	if (LeftCharacterFullProfileImage)
		LeftCharacterFullProfileImage->SetVisibility(ESlateVisibility::Collapsed);

	if (RightCharacterFullProfileImage)
		RightCharacterFullProfileImage->SetVisibility(ESlateVisibility::Collapsed);

	if (CharacterList)
	{
		if (auto* ListItemData = CharacterList->GetItemAt(0))
			CharacterList->SetSelectedItem(ListItemData);
	}

	if (ListStateSwitcher)
		ListStateSwitcher->SetActiveWidgetIndex(0);
}

void UThrCharacterList::SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime)
{
	MatchState = InMatchState;
	MatchStateUpdateTime = InMatchStateUpdateTime;

	if (MatchState == ThrMatchState::InProgress)
	{
		if (LockInButton)
			LockInButton->SetIsEnabled(SelectedListItemData.IsValid());

		if (!bInit)
		{
			bInit = true;
			bLeftSide ? PlayAnimInProgressLeft() : PlayAnimInProgressRight();
		}
	}
	else if (MatchState == ThrMatchState::PrepareBattle)
	{
		auto* EnemyCharacterFullProfileImage = (bLeftSide ? RightCharacterFullProfileImage : LeftCharacterFullProfileImage);
		if (EnemyCharacterFullProfileImage && GAMEDATA)
		{
			const auto& EnemyCharacterData = GAMEDATA->GetCharacterData(EnemyCharacterName);

			EnemyCharacterFullProfileImage->SetBrushFromTexture(EnemyCharacterData.FullProfileImage);
			EnemyCharacterFullProfileImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		PlayAnimPrepareBattle();
	}
	else if (MatchState == ThrMatchState::StartBattle)
	{
		Init();
		PlayAnimStartBattle();
	}
}

void UThrCharacterList::SetEnemyTeamData(const FName InEnemyTeamName)
{
	auto* EnemyCharacterFullProfileBorder = (bLeftSide ? RightCharacterFullProfileBorder : LeftCharacterFullProfileBorder);
	if (EnemyCharacterFullProfileBorder && GAMEDATA)
	{
		const auto& EnemyTeamData = GAMEDATA->GetTeamData(InEnemyTeamName);
		EnemyCharacterFullProfileBorder->SetColorAndOpacity(EnemyTeamData.MainColor);
	}
}

void UThrCharacterList::SetEnemyCharacterData(const FName InEnemyCharacterName)
{
	EnemyCharacterName = InEnemyCharacterName;
}

void UThrCharacterList::ClickLockInButton()
{
	if (bLockIn || !SelectedListItemData.IsValid())
		return;

	bLockIn = true;

	if (auto* PlayerState = GetOwningPlayerState<AThrPlayerState>())
		PlayerState->ServerSetCharacterName(SelectedListItemData->CharacterName);

	if (auto* Player = GetOwningPlayer<AThrPlayerController>())
		Player->ServerTryToSetBattleState(ThrMatchState::PrepareBattle);

	if (GAMEDATA)
	{
		const FName CharacterName = SelectedListItemData->CharacterName;

		const auto& TeamData = GAMEDATA->GetTeamData(TeamName);
		const auto& CharacterData = GAMEDATA->GetCharacterData(CharacterName);
		
		if (CharacterProfileImage)
		{
			CharacterProfileImage->SetBrushFromTexture(CharacterData.ProfileImage);
			CharacterProfileImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
		}

		if (CharacterNameText)
			CharacterNameText->SetText(FText::FromName(CharacterName));

		if (CharacterContractWithText)
			CharacterContractWithText->SetText(FText::FromString(TEXT("Contract with: ") + CharacterData.ContractWith));

		if (CharacterDetailsText)
			CharacterDetailsText->SetText(FText::FromString(CharacterData.Details));

		if (ListStateSwitcher)
			ListStateSwitcher->SetActiveWidgetIndex(1);
	}
}

void UThrCharacterList::SelectCharacterListItem(UObject* InListItemData)
{
	SelectedListItemData = Cast<UThrCharacterListItemData>(InListItemData);

	auto* CharacterFullProfileImage = (bLeftSide ? LeftCharacterFullProfileImage : RightCharacterFullProfileImage);
	if (CharacterFullProfileImage && GAMEDATA)
	{
		const auto& CharacterData = GAMEDATA->GetCharacterData(SelectedListItemData->CharacterName);

		CharacterFullProfileImage->SetBrushFromTexture(CharacterData.FullProfileImage);
		CharacterFullProfileImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (LockInButton && MatchState == ThrMatchState::InProgress)
		LockInButton->SetIsEnabled(SelectedListItemData.IsValid());
}
