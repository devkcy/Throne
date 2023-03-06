#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrCharacterList.generated.h"

UCLASS()
class THRONE_API UThrCharacterList : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& InGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInitLeft();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInitRight();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInProgressLeft();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInProgressRight();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimPrepareBattle();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimStartBattle();

private:
	void Init();

	void SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime);
	void SetEnemyTeamData(const FName InEnemyTeamName);
	void SetEnemyCharacterData(const FName InEnemyCharacterName);

	UFUNCTION()
	void ClickLockInButton();

	void SelectCharacterListItem(UObject* InListItemData);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* ListStateSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* LockInButton;

	UPROPERTY(meta = (BindWidget))
	class UTileView* CharacterList;

	UPROPERTY(meta = (BindWidget))
	class UImage* CharacterProfileImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterContractWithText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterDetailsText;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftCharacterFullProfileImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftCharacterFullProfileBorder;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightCharacterFullProfileImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightCharacterFullProfileBorder;

private:
	bool bInit;
	bool bLeftSide;

	FName TeamName;
	FName EnemyCharacterName;
	
	FName MatchState;
	FDateTime MatchStateUpdateTime;

	bool bLockIn;
	TWeakObjectPtr<class UThrCharacterListItemData> SelectedListItemData;
};
