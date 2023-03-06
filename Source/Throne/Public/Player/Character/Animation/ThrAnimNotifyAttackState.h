#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ThrAnimNotifyAttackState.generated.h"

UCLASS()
class THRONE_API UThrAnimNotifyAttackState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, float InTotalDuration, const FAnimNotifyEventReference& InEventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, float InFrameDeltaTime, const FAnimNotifyEventReference& InEventReference) override;

private:
	TWeakObjectPtr<class AThrCharacter> Owner;
	FHitResult HitInfo;

	const ETraceTypeQuery WeaponTrace = TraceTypeQuery3;
};
