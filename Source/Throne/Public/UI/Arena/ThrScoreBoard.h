#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrScoreBoard.generated.h"

UCLASS()
class THRONE_API UThrScoreBoard : public UUserWidget
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

private:
	void SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime);
	void SetEnemyTeamData(const FName InEnemyTeamName);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ProgressText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LeftTeamNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LeftScoreText;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftTeamEmblemImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftTeamEmblemShadeImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftTeamEmblemStrokeImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RightTeamNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RightScoreText;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightTeamEmblemImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightTeamEmblemShadeImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightTeamEmblemStrokeImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightBackgroundImage;

private:
	bool bInit;
	bool bLeftSide;

	FDateTime MatchStateUpdateTime;
};
