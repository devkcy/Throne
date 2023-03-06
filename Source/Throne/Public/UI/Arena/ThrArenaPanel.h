#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrArenaPanel.generated.h"

UCLASS()
class THRONE_API UThrArenaPanel : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimNotifyMessage();
	
	UFUNCTION(BlueprintCallable)
	void ReplayAnimNotifyMessage();
	
	UFUNCTION(BlueprintCallable)
	void TryToSetBattleState(const FName InBattleState);
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInit();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimInProgress();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimPrepareBattle();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimStartBattle();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimEndBattle();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimLeavingMap();

private:
	void NotifyMessage(const FString& InMessage);

	void SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime);
	void SetEnemyTeamData(const FName InEnemyTeamName);
	void EndBattle(const int32 InBattleResult);

	void InputSurrender();
	void Surrender(const FString& InMessage);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NotifyText;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftTeamFlagImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightTeamFlagImage;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MatchStateSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* BattleResultSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MatchResultSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UThrDialog* Dialog;

private:
	bool bLeftSide;

	TArray<FString> Messages;
};
