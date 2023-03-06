#include "Player/Character/Animation/ThrAnimInstance.h"
#include "Gamedata/ThrDataManager.h"
#include "Player/Character/ThrCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void UThrAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OnMontageEnded.AddDynamic(this, &UThrAnimInstance::EndAction);

	Owner = Cast<AThrCharacter>(GetOwningActor());
	if (Owner.IsValid())
	{
		Owner->OnPlayAction.BindUObject(this, &UThrAnimInstance::PlayAction);

		OwnerMovement = Owner->GetCharacterMovement();
		CurWorldLocation = Owner->GetActorLocation();
	}
}

void UThrAnimInstance::NativeUpdateAnimation(float InDeltaSeconds)
{
	Super::NativeUpdateAnimation(InDeltaSeconds);

	if (!Owner.IsValid() || !OwnerMovement)
		return;

	bHasAcceleration = !OwnerMovement->GetCurrentAcceleration().IsNearlyZero();
	bHasVelocity = !OwnerMovement->Velocity.IsNearlyZero();
	
	AngleWithVelocity = CalculateDirection(OwnerMovement->Velocity.GetSafeNormal2D(), Owner->GetActorRotation());
	const float AbsAngleWithVelocity = FMath::Abs(AngleWithVelocity);
	
	MovementDirection = (AngleWithVelocity > 0.f ? MoveRight : MoveLeft);
	MovementDirection = (AbsAngleWithVelocity < 60.f ? MoveForward : (AbsAngleWithVelocity > 120.f ? MoveBackward : MovementDirection));

	DistanceTraveled = (Owner->GetActorLocation() - CurWorldLocation).Size2D();
	CurWorldLocation = Owner->GetActorLocation();

	if (DistanceTraveled)
		DistanceTraveledSpeed = DistanceTraveled / InDeltaSeconds;
}

void UThrAnimInstance::EndAction(UAnimMontage* InMontage, bool bInInterrupted)
{
	if (Owner.IsValid())
		Owner->EndAction();
}

void UThrAnimInstance::PlayAction(const EThrActionType InAction)
{
	if (!Owner.IsValid() || !GAMEDATA)
		return;

	const auto& CharacterAnimData = GAMEDATA->GetCharacterAnimData(Owner->GetCharacterName());
	if (auto* Montage = CharacterAnimData.AnimsByAction.Find(InAction))
		Montage_Play(*Montage);
}
