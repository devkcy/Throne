#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrDialog.generated.h"

enum EThrDialogType
{
	Confirm,
	InputText,
	CheckList
};

struct FThrDialogInfo
{
public:
	FString TitleMessage;
	EThrDialogType DialogType;
	FString ExtraMessage;
	FString InputHintMessage;
	TArray<TPair<FString, FLinearColor>> CheckListItemsData;
};

DECLARE_DELEGATE_OneParam(FOnSubmitDialogInfo, const FString&);

UCLASS()
class THRONE_API UThrDialog : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void Show(const FThrDialogInfo& InDialogInfo);
	
private:
	UFUNCTION()
	void ClickCancelButton();

	UFUNCTION()
	void ClickOkButton();

	UFUNCTION()
	void ChangeInputText(const FText& InInputText);

	void SelectCheckListItem(UObject* InListItemData);

public:
	FOnSubmitDialogInfo OnSubmitDialogInfo;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* OkButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* InfoSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ExtraText;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* InputText;

	UPROPERTY(meta = (BindWidget))
	class UListView* CheckList;

private:
	FThrDialogInfo DialogInfo;

	TWeakObjectPtr<class UThrCheckListItemData> SelectedListItemData;
};
