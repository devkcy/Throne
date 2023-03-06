#include "UI/Lobby/ThrLobbyPanel.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"
#include "UI/ThrDialog.h"
#include "UI/Lobby/ThrMatchList.h"

#include "Components/TextBlock.h"
#include "Online.h"

void UThrLobbyPanel::NativeConstruct()
{
	Super::NativeConstruct();

	UThrGameInstance::OnNotifyLobbyMessage.BindUObject(this, &UThrLobbyPanel::NotifyMessage);

	if (MatchList)
	{
		MatchList->OnClickCreateSessionButton.BindUObject(this, &UThrLobbyPanel::ClickCreateSessionButton);
		MatchList->OnClickJoinSessionButton.BindUObject(this, &UThrLobbyPanel::ClickJoinSessionButton);
	}
}

void UThrLobbyPanel::ReplayAnimNotifyMessage()
{
	if (Messages.IsValidIndex(0))
		Messages.RemoveAt(0);

	if (NotifyText && Messages.IsValidIndex(0))
	{
		NotifyText->SetText(FText::FromString(Messages[0]));
		PlayAnimNotifyMessage();
	}
}

void UThrLobbyPanel::NotifyMessage(const FString& InMessage)
{
	if (!NotifyText || Messages.Add(InMessage))
		return;

	NotifyText->SetText(FText::FromString(InMessage));
	PlayAnimNotifyMessage();
}

void UThrLobbyPanel::ClickCreateSessionButton()
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Please input a new match title");
	DialogInfo.DialogType = InputText;
	DialogInfo.InputHintMessage = TEXT("New match");

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::SetSessionName);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::SetSessionName(const FString& InSessionName)
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	if (InSessionName.IsEmpty())
		return;

	SessionSettingParams.FindOrAdd(TEXT("SessionName")) = InSessionName;

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Please choose your side");
	DialogInfo.DialogType = CheckList;
	DialogInfo.CheckListItemsData.Emplace(TEXT("Left"), FLinearColor(0.4f, 0.f, 0.f));
	DialogInfo.CheckListItemsData.Emplace(TEXT("Right"), FLinearColor(0.f, 0.1f, 0.5f));
	
	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::SetSide);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::SetSide(const FString& InSide)
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	if (InSide.IsEmpty())
		return;

	SessionSettingParams.FindOrAdd(TEXT("Side")) = InSide;

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Please choose your team");
	DialogInfo.DialogType = CheckList;

	if (GAMEDATA)
	{
		const auto& TeamsData = GAMEDATA->GetAllTeamsData();
		for (const auto& TeamData : TeamsData)
			DialogInfo.CheckListItemsData.Emplace(TeamData.Key.ToString(), TeamData.Value.MainColor);
	}

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::SetCreateTeamName);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::SetCreateTeamName(const FString& InTeamName)
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	if (InTeamName.IsEmpty())
		return;

	SessionSettingParams.FindOrAdd(TEXT("TeamName")) = InTeamName;

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Are you sure to create the match?");
	DialogInfo.DialogType = Confirm;

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::CreateSession);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::CreateSession(const FString& InAnswer)
{
	if (Dialog)
		Dialog->OnSubmitDialogInfo.Unbind();

	if (InAnswer.IsEmpty())
		return;

	if (auto* GameInstance = GetGameInstance<UThrGameInstance>())
	{
		const auto& KST = FDateTime::UtcNow() + FTimespan::FromHours(9);
		SessionSettingParams.FindOrAdd(TEXT("CreationTime")) = KST.ToFormattedString(TEXT("%H:%M"));

		GameInstance->TryToCreateSession(SessionSettingParams);
		return;
	}

	NotifyMessage(TEXT("Failed to create the match"));
}

void UThrLobbyPanel::ClickJoinSessionButton(const FName InEnemyTeamName)
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Please choose your team");
	DialogInfo.DialogType = CheckList;

	if (GAMEDATA)
	{
		const auto& TeamsData = GAMEDATA->GetAllTeamsData();
		for (const auto& TeamData : TeamsData)
		{
			if (TeamData.Key != InEnemyTeamName)
				DialogInfo.CheckListItemsData.Emplace(TeamData.Key.ToString(), TeamData.Value.MainColor);
		}
	}

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::SetJoinTeamName);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::SetJoinTeamName(const FString& InTeamName)
{
	if (!Dialog)
		return;

	Dialog->OnSubmitDialogInfo.Unbind();

	if (InTeamName.IsEmpty())
		return;

	TeamName = FName(InTeamName);

	FThrDialogInfo DialogInfo;
	DialogInfo.TitleMessage = TEXT("Are you sure to join the match?");
	DialogInfo.DialogType = Confirm;

	Dialog->OnSubmitDialogInfo.BindUObject(this, &UThrLobbyPanel::JoinSession);
	Dialog->Show(DialogInfo);
}

void UThrLobbyPanel::JoinSession(const FString& InAnswer)
{
	if (Dialog)
		Dialog->OnSubmitDialogInfo.Unbind();

	if (InAnswer.IsEmpty())
		return;

	if (MatchList)
	{
		auto* GameInstance = Cast<UThrGameInstance>(GetGameInstance());
		const auto& SessionSearchResult = MatchList->GetSessionSearchResult();
		if (GameInstance && SessionSearchResult.IsValid())
		{
			GameInstance->TryToJoinSession(MatchList->GetSessionSearchResult(), TeamName);
			return;
		}
	}

	NotifyMessage(TEXT("Failed to join the match"));
}
