#include "UI/Lobby/ThrMatchList.h"
#include "Gameplay/ThrGameInstance.h"
#include "UI/Lobby/ThrMatchListItem.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/WidgetSwitcher.h"

void UThrMatchList::NativeConstruct()
{
	Super::NativeConstruct();

	UThrGameInstance::OnFindSessions.BindUObject(this, &UThrMatchList::FindSessions);

	if (CreateSessionButton)
		CreateSessionButton->OnClicked.AddDynamic(this, &UThrMatchList::ClickCreateSessionButton);

	if (FindSessionsButton)
		FindSessionsButton->OnClicked.AddDynamic(this, &UThrMatchList::ClickFindSessionsButton);

	if (JoinSessionButton)
		JoinSessionButton->OnClicked.AddDynamic(this, &UThrMatchList::ClickJoinSessionButton);

	if (MatchList)
		MatchList->OnItemSelectionChanged().AddUObject(this, &UThrMatchList::SelectMatchListItem);

	ClickFindSessionsButton();
}

const FOnlineSessionSearchResult UThrMatchList::GetSessionSearchResult() const
{
	if (SelectedListItemData.IsValid())
		return SelectedListItemData->SessionSearchResult;

	return FOnlineSessionSearchResult();
}

void UThrMatchList::FindSessions(bool bInSucceeded)
{
	if (MatchList)
	{
		SelectedListItemData.Reset();
		MatchList->ClearListItems();

		auto* GameInstance = GetGameInstance<UThrGameInstance>();
		if (bInSucceeded && GameInstance)
		{
			const auto& SessionSearchResults = GameInstance->GetSessionSearchResults();
			for (const auto& SessionSearchResult : SessionSearchResults)
			{
				if (auto* ListItemData = NewObject<UThrMatchListItemData>())
				{
					ListItemData->SessionSearchResult = SessionSearchResult;

					MatchList->AddItem(ListItemData);
				}
			}

			if (auto* ListItemData = MatchList->GetItemAt(0))
				MatchList->SetSelectedItem(ListItemData);
		}
	}

	if (ListStateSwitcher)
		ListStateSwitcher->SetActiveWidgetIndex(MatchList->GetNumItems() > 0 ? 0 : 1);

	if (FindSessionsButton)
		FindSessionsButton->SetIsEnabled(true);
}

void UThrMatchList::ClickCreateSessionButton()
{
	OnClickCreateSessionButton.ExecuteIfBound();
}

void UThrMatchList::ClickFindSessionsButton()
{
	if (FindSessionsButton)
		FindSessionsButton->SetIsEnabled(false);

	if (JoinSessionButton)
		JoinSessionButton->SetIsEnabled(false);

	if (ListStateSwitcher)
		ListStateSwitcher->SetActiveWidgetIndex(2);

	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		GameInstance->TryToFindSessions();
		return;
	}

	FindSessions(false);
}

void UThrMatchList::ClickJoinSessionButton()
{
	if (SelectedListItemData.IsValid())
	{
		if (const auto* TeamName = SelectedListItemData->SessionSearchResult.Session.SessionSettings.Settings.Find(TEXT("TeamName")))
		{
			OnClickJoinSessionButton.ExecuteIfBound(FName(TeamName->Data.ToString()));
			return;
		}
	}

	UThrGameInstance::OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Failed to join the match"));
}

void UThrMatchList::SelectMatchListItem(UObject* InListItemData)
{
	SelectedListItemData = Cast<UThrMatchListItemData>(InListItemData);

	if (JoinSessionButton)
		JoinSessionButton->SetIsEnabled(SelectedListItemData.IsValid());
}
