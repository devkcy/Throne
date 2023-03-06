#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrLobbyPanel.generated.h"

UCLASS()
class THRONE_API UThrLobbyPanel : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimNotifyMessage();

	UFUNCTION(BlueprintCallable)
	void ReplayAnimNotifyMessage();

private:
	void NotifyMessage(const FString& InMessage);

	void ClickCreateSessionButton();
	void SetSessionName(const FString& InSessionName);
	void SetSide(const FString& InSide);
	void SetCreateTeamName(const FString& InTeamName);
	void CreateSession(const FString& InAnswer);

	void ClickJoinSessionButton(const FName InEnemyTeamName);
	void SetJoinTeamName(const FString& InTeamName);
	void JoinSession(const FString& InAnswer);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NotifyText;

	UPROPERTY(meta = (BindWidget))
	class UThrMatchList* MatchList;

	UPROPERTY(meta = (BindWidget))
	class UThrDialog* Dialog;

private:
	TArray<FString> Messages;

	TMap<FName, FString> SessionSettingParams;
	FName TeamName;
};
