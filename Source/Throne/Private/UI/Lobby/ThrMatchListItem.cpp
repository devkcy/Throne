#include "UI/Lobby/ThrMatchListItem.h"
#include "Gamedata/ThrDataManager.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UThrMatchListItem::NativeOnListItemObjectSet(UObject* InListItemData)
{
	auto* ListItemData = Cast<UThrMatchListItemData>(InListItemData);
	if (!ListItemData)
		return;

	const auto& SessionSettings = ListItemData->SessionSearchResult.Session.SessionSettings.Settings;

	if (SessionNameText)
	{
		if (auto* SessionName = SessionSettings.Find(TEXT("SessionName")))
			SessionNameText->SetText(FText::FromString(SessionName->Data.ToString()));
	}

	if (SideText)
	{
		if (auto* Side = SessionSettings.Find(TEXT("Side")))
		{
			const auto& SideStr = Side->Data.ToString();
			const auto& SideColor = (SideStr == TEXT("Left") ? FLinearColor(0.4f, 0.f, 0.f) : FLinearColor(0.f, 0.1f, 0.5f));

			SideText->SetText(FText::FromString(SideStr));
			SideText->SetColorAndOpacity(SideColor);
		}
	}

	if (TeamNameText && GAMEDATA)
	{
		if (auto* TeamName = SessionSettings.Find(TEXT("TeamName")))
		{
			const auto& TeamNameStr = TeamName->Data.ToString();
			const auto& TeamData = GAMEDATA->GetTeamData(FName(TeamNameStr));
			
			TeamNameText->SetText(FText::FromString(TeamNameStr));
			TeamNameText->SetColorAndOpacity(TeamData.MainColor);
		}
	}

	if (CreationTimeText)
	{
		if (auto* CreationTime = SessionSettings.Find(TEXT("CreationTime")))
			CreationTimeText->SetText(FText::FromString(CreationTime->Data.ToString()));
	}

	if (FocusBorder)
		FocusBorder->SetVisibility(ESlateVisibility::Collapsed);
}

void UThrMatchListItem::NativeOnItemSelectionChanged(bool bInSelected)
{
	if (FocusBorder)
		FocusBorder->SetVisibility(bInSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
