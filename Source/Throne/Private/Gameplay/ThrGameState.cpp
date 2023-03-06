#include "Gameplay/ThrGameState.h"

#include "Net/UnrealNetwork.h"

void AThrGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrGameState, LeftScore);
	DOREPLIFETIME(AThrGameState, RightScore);
}
