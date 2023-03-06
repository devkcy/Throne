#include "UI/ThrDialog.h"
#include "UI/ThrCheckListItem.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void UThrDialog::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UThrDialog::ClickCancelButton);

	if (CancelButton)
		CancelButton->OnClicked.AddDynamic(this, &UThrDialog::ClickCancelButton);

	if (OkButton)
		OkButton->OnClicked.AddDynamic(this, &UThrDialog::ClickOkButton);

	if (InputText)
		InputText->OnTextChanged.AddDynamic(this, &UThrDialog::ChangeInputText);

	if (CheckList)
		CheckList->OnItemSelectionChanged().AddUObject(this, &UThrDialog::SelectCheckListItem);
}

void UThrDialog::Show(const FThrDialogInfo& InDialogInfo)
{
	DialogInfo = InDialogInfo;

	if (OkButton)
		OkButton->SetIsEnabled(DialogInfo.DialogType == EThrDialogType::Confirm);

	if (TitleText)
		TitleText->SetText(FText::FromString(DialogInfo.TitleMessage));

	if (InfoSwitcher)
		InfoSwitcher->SetActiveWidgetIndex(DialogInfo.DialogType);

	if (ExtraText)
		ExtraText->SetText(FText::FromString(DialogInfo.ExtraMessage));

	if (InputText)
	{
		InputText->SetText(FText::FromString(TEXT("")));
		InputText->SetHintText(FText::FromString(DialogInfo.InputHintMessage));
	}

	if (CheckList)
	{
		SelectedListItemData.Reset();
		CheckList->ClearListItems();

		for (const auto& CheckListItemData : DialogInfo.CheckListItemsData)
		{
			if (auto* ListItemData = NewObject<UThrCheckListItemData>())
			{
				ListItemData->CheckBoxText = CheckListItemData.Key;
				ListItemData->TextColor = CheckListItemData.Value;

				CheckList->AddItem(ListItemData);
			}
		}

		if (auto* ListItemData = CheckList->GetItemAt(0))
			CheckList->SetSelectedItem(ListItemData);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UThrDialog::ClickCancelButton()
{
	SetVisibility(ESlateVisibility::Collapsed);

	OnSubmitDialogInfo.ExecuteIfBound(TEXT(""));
}

void UThrDialog::ClickOkButton()
{
	SetVisibility(ESlateVisibility::Collapsed);

	FString SubmitDialogInfo;
	switch (DialogInfo.DialogType)
	{
	case EThrDialogType::Confirm:
		SubmitDialogInfo = TEXT("Ok");
		break;
	case EThrDialogType::InputText:
		SubmitDialogInfo = (InputText ? InputText->GetText().ToString() : TEXT(""));
		break;
	case EThrDialogType::CheckList:
		SubmitDialogInfo = (SelectedListItemData.IsValid() ? SelectedListItemData->CheckBoxText : TEXT(""));
		break;
	}

	OnSubmitDialogInfo.ExecuteIfBound(SubmitDialogInfo);
}

void UThrDialog::ChangeInputText(const FText& InInputText)
{
	if (DialogInfo.DialogType != EThrDialogType::InputText)
		return;

	if (OkButton)
		OkButton->SetIsEnabled(!InInputText.IsEmptyOrWhitespace());
}

void UThrDialog::SelectCheckListItem(UObject* InListItemData)
{
	if (DialogInfo.DialogType != EThrDialogType::CheckList)
		return;

	SelectedListItemData = Cast<UThrCheckListItemData>(InListItemData);

	if (OkButton)
		OkButton->SetIsEnabled(SelectedListItemData.IsValid());
}
