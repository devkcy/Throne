#include "Player/Character/Animation/ThrAnimNotifyAttackState.h"
#include "Player/Character/ThrCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UThrAnimNotifyAttackState::NotifyBegin(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, float InTotalDuration, const FAnimNotifyEventReference& InEventReference)
{
	Owner = Cast<AThrCharacter>(InMeshComp->GetOwner());
	HitInfo.Reset();
}

void UThrAnimNotifyAttackState::NotifyTick(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, float InFrameDeltaTime, const FAnimNotifyEventReference& InEventReference)
{
	if (!Owner.IsValid() || !Owner->GetWeapon().IsValid() || Cast<AThrCharacter>(HitInfo.GetActor()) || !InMeshComp)
		return;

	const auto& Weapon = Owner->GetWeapon();
	const auto& WeaponStartPoint = Weapon->GetSocketLocation(TEXT("Start"));
	const auto& WeaponEndPoint = Weapon->GetSocketLocation(TEXT("End"));

	if (UKismetSystemLibrary::LineTraceSingle(InMeshComp, WeaponStartPoint, WeaponEndPoint, WeaponTrace, true, TArray<AActor*>(), EDrawDebugTrace::None, HitInfo, true))
	{
		if (Cast<AThrCharacter>(HitInfo.GetActor()))
			Owner->ApplyDamageToEnemy(HitInfo);
	}
}
