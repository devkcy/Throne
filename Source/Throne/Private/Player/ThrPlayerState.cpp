#include "Player/ThrPlayerState.h"

#include "Net/UnrealNetwork.h"

void AThrPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrPlayerState, bLeftSide);
	DOREPLIFETIME(AThrPlayerState, TeamName);
	DOREPLIFETIME(AThrPlayerState, CharacterName);
	DOREPLIFETIME(AThrPlayerState, MaxSP);
	DOREPLIFETIME(AThrPlayerState, CurSP);
	DOREPLIFETIME(AThrPlayerState, bDead);
	DOREPLIFETIME(AThrPlayerState, LastTakeDamageTime);
	DOREPLIFETIME(AThrPlayerState, BattleResult);
	DOREPLIFETIME(AThrPlayerState, bSurrender);
}

bool AThrPlayerState::ServerSetCharacterName_Validate(const FName InCharacterName)
{
	RPC_VALIDATE(InCharacterName.IsValid());
	return true;
}

void AThrPlayerState::ServerSetCharacterName_Implementation(const FName InCharacterName)
{
	CharacterName = InCharacterName;
}

void AThrPlayerState::ServerSetSurrender_Implementation(const bool bInSurrender)
{
	bSurrender = bInSurrender;
}
