#include "UI/ThrCheckListItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UThrCheckListItem::NativeOnListItemObjectSet(UObject* InListItemData)
{
	auto* ListItemData = Cast<UThrCheckListItemData>(InListItemData);
	if (!ListItemData)
		return;

	if (CheckBoxText)
	{
		CheckBoxText->SetText(FText::FromString(ListItemData->CheckBoxText));
		CheckBoxText->SetColorAndOpacity(ListItemData->TextColor);
	}

	if (CheckImage)
		CheckImage->SetVisibility(ESlateVisibility::Hidden);
}

void UThrCheckListItem::NativeOnItemSelectionChanged(bool bInSelected)
{
	if (CheckImage)
		CheckImage->SetVisibility(bInSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
}
