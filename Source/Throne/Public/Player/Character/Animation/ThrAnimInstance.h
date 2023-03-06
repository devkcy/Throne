#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ThrAnimInstance.generated.h"

UENUM(BlueprintType)
enum EThrMovementDirectionType
{
	MoveForward,
	MoveBackward,
	MoveLeft,
	MoveRight
};

UCLASS()
class THRONE_API UThrAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float InDeltaSeconds) override;

private:
	UFUNCTION()
	void EndAction(UAnimMontage* InMontage, bool bInInterrupted);
	void PlayAction(const enum EThrActionType InAction);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	bool bHasAcceleration;

	UPROPERTY(BlueprintReadOnly)
	bool bHasVelocity;
	
	UPROPERTY(BlueprintReadOnly)
	float AngleWithVelocity;

	UPROPERTY(BlueprintReadOnly)
	float DistanceTraveled;

	UPROPERTY(BlueprintReadOnly)
	float DistanceTraveledSpeed;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EThrMovementDirectionType> MovementDirection;

	UPROPERTY(BlueprintReadOnly)
	class UCharacterMovementComponent* OwnerMovement;

private:
	FVector CurWorldLocation;

	TWeakObjectPtr<class AThrCharacter> Owner;
};
