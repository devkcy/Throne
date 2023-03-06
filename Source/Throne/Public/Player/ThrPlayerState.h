#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ThrPlayerState.generated.h"

UCLASS()
class THRONE_API AThrPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCharacterName(const FName InCharacterName);

	UFUNCTION(Server, Reliable)
	void ServerSetSurrender(const bool bInSurrender);

	UPROPERTY(Replicated)
	bool bLeftSide;

	UPROPERTY(Replicated)
	FName TeamName;

	UPROPERTY(Replicated)
	FName CharacterName;

	UPROPERTY(Replicated)
	float MaxSP;

	UPROPERTY(Replicated)
	float CurSP;

	UPROPERTY(Replicated)
	bool bDead;

	UPROPERTY(Replicated)
	FDateTime LastTakeDamageTime;

	UPROPERTY(Replicated)
	int32 BattleResult;

	UPROPERTY(Replicated)
	bool bSurrender;
};
