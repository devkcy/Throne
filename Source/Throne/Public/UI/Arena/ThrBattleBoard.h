#pragma once

#include "../../Gamedata/ThrCharacterData.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrBattleBoard.generated.h"

UCLASS()
class THRONE_API UThrBattleBoard : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& InGeometry, float InDeltaTime) override;

private:
	void UpdateSP(const bool bLeftSide, const float InMaxSP, const float InCurSP);

	void SetMatchState(const FName InMatchState, const FDateTime InMatchStateUpdateTime);
	void SetEnemyTeamData(const FName InEnemyTeamName);
	void EndBattle(const int32 InBattleResult);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RoundNumText;

	UPROPERTY(meta = (BindWidget))
	class UImage* Round1PointImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* Round2PointImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* Round3PointImage;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* LeftSpBar;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* LeftRapierImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeftPlayerImage;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* RightSpBar;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightRapierImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* RightPlayerImage;

private:
	UPROPERTY()
	TArray<UImage*> RoundPointImages;

	bool bLeftSide;

	FName TeamName;
	FName EnemyTeamName;

	int32 TotalScore;

	float LeftMaxSP;
	float LeftCurSP;

	float RightMaxSP;
	float RightCurSP;
};
