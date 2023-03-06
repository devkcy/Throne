#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrMatchList.generated.h"

DECLARE_DELEGATE(FOnClickCreateSessionButton);
DECLARE_DELEGATE_OneParam(FOnClickJoinSessionButton, const FName);

UCLASS()
class THRONE_API UThrMatchList : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	const FOnlineSessionSearchResult GetSessionSearchResult() const;

private:
	void FindSessions(const bool bInSucceeded);

	UFUNCTION()
	void ClickCreateSessionButton();

	UFUNCTION()
	void ClickFindSessionsButton();
	
	UFUNCTION()
	void ClickJoinSessionButton();
	
	void SelectMatchListItem(UObject* InListItemData);
	
public:
	FOnClickCreateSessionButton OnClickCreateSessionButton;
	FOnClickJoinSessionButton OnClickJoinSessionButton;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* CreateSessionButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* FindSessionsButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinSessionButton;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* ListStateSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UListView* MatchList;

private:
	TWeakObjectPtr<class UThrMatchListItemData> SelectedListItemData;
};
