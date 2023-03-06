#include "UI/Arena/ThrCharacterListItem.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameInstance.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UThrCharacterListItem::NativeOnListItemObjectSet(UObject* InListItemData)
{
	auto* ListItemData = Cast<UThrCharacterListItemData>(InListItemData);
	if (!ListItemData)
		return;

	if (CharacterNameText)
		CharacterNameText->SetText(FText::FromName(ListItemData->CharacterName));

	if (CharacterProfileImage && GAMEDATA)
	{
		const auto& CharacterData = GAMEDATA->GetCharacterData(ListItemData->CharacterName);
		
		CharacterProfileImage->SetBrushFromTexture(CharacterData.ProfileImage);
		CharacterProfileImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));
	}

	if (FocusBorder)
		FocusBorder->SetVisibility(ESlateVisibility::Collapsed);
}

void UThrCharacterListItem::NativeOnItemSelectionChanged(bool bInSelected)
{
	if (FocusBorder)
		FocusBorder->SetVisibility(bInSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
