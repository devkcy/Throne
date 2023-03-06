#include "Player/Character/ThrCharacter.h"
#include "Gameplay/ThrCameraManager.h"
#include "Gameplay/ThrGameState.h"
#include "Player/ThrPlayerController.h"
#include "Player/ThrPlayerState.h"
#include "Player/Character/ThrSkinnedDecalComponent.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

FOnUpdateSP AThrCharacter::OnUpdateSP;

AThrCharacter::AThrCharacter()
	: Weapon(CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon")))
	, BloodDecal(CreateDefaultSubobject<UThrSkinnedDecalComponent>(TEXT("BloodDecal")))
	, MeshPhysicalAnim(CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("MeshPhysicalAnim")))
	, WeaponPhysicalAnim(CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("WeaponPhysicalAnim")))
	, WeaponPhysicsConstraint(CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("WeaponPhysicsConstraint")))
{
	SetActorTickInterval(0.1f);

	GetMesh()->SetCollisionProfileName(TEXT("Character"));
	
	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	if (Weapon)
	{
		Weapon->SetupAttachment(GetMesh(), TEXT("Weapon"));
		Weapon->SetCollisionProfileName(TEXT("Weapon"));
		Weapon->SetAllBodiesNotifyRigidBodyCollision(true);
	}

	if (WeaponPhysicsConstraint)
	{
		WeaponPhysicsConstraint->SetupAttachment(Weapon);
		WeaponPhysicsConstraint->SetConstrainedComponents(GetMesh(), TEXT("hand_r"), Weapon, TEXT("Bone_002"));
	}
}

void AThrCharacter::BeginPlay()
{
	Super::BeginPlay();

	AThrPlayerController::OnClientEndBattle.AddUObject(this, &AThrCharacter::ServerEndBattle);

	FPhysicalAnimationData PhysicalAnimData;
	PhysicalAnimData.bIsLocalSimulation = false;
	PhysicalAnimData.PositionStrength = 1000.f;
	PhysicalAnimData.OrientationStrength = 1000.f;
	
	if (MeshPhysicalAnim)
	{
		PhysicalAnimData.VelocityStrength = 100.f;
		PhysicalAnimData.AngularVelocityStrength = 100.f;

		MeshPhysicalAnim->SetSkeletalMeshComponent(GetMesh());
		MeshPhysicalAnim->ApplyPhysicalAnimationSettingsBelow(TEXT("pelvis"), PhysicalAnimData, false);

		GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, false);
	}

	if (Weapon && WeaponPhysicalAnim)
	{
		Weapon->OnComponentHit.AddDynamic(this, &AThrCharacter::ClientSparkEffect);

		WeaponPhysicalAnim->SetSkeletalMeshComponent(Weapon);
		WeaponPhysicalAnim->ApplyPhysicalAnimationSettingsBelow(TEXT("Bone"), PhysicalAnimData, false);

		Weapon->SetAllBodiesBelowSimulatePhysics(TEXT("Bone"), true, false);
	}
}

void AThrCharacter::Tick(float InDeltaSeconds)
{
	Super::Tick(InDeltaSeconds);

	if (HasAuthority())
	{
		auto* OwningPlayerState = GetPlayerState<AThrPlayerState>();
		if (OwningPlayerState)
		{
			const float IncreaseSP = OwningPlayerState->CurSP + 10.f * InDeltaSeconds;
			OwningPlayerState->CurSP = FMath::Min(IncreaseSP, OwningPlayerState->MaxSP);
		}
	}
}

void AThrCharacter::SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent)
{
	Super::SetupPlayerInputComponent(InPlayerInputComponent);

	if (auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InPlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputActions.FindRef(MoveInput), ETriggerEvent::Triggered, this, &AThrCharacter::Move);
		
		EnhancedInputComponent->BindAction(InputActions.FindRef(AttackTopInput), ETriggerEvent::Started, this, &AThrCharacter::SetAction, AttackTop, true);
		EnhancedInputComponent->BindAction(InputActions.FindRef(AttackTopInput), ETriggerEvent::Completed, this, &AThrCharacter::SetAction, AttackTop, false);
	}
}

void AThrCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrCharacter, bLeftSide);
	DOREPLIFETIME(AThrCharacter, CharacterName);
}

void AThrCharacter::PossessedBy(AController* InNewController)
{
	Super::PossessedBy(InNewController);

	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
	{
		bLeftSide = OwningPlayerState->bLeftSide;
		CharacterName = OwningPlayerState->CharacterName;

		OwningPlayerState->MaxSP = OwningPlayerState->CurSP = 100.f;
		OwningPlayerState->bDead = false;
	}

	MulticastSetCamera();
}

void AThrCharacter::ApplyDamageToEnemy(const FHitResult& InHitInfo)
{
	if (!HasAuthority())
		return;

	if (auto* EnemyCharacter = Cast<AThrCharacter>(InHitInfo.GetActor()))
		EnemyCharacter->TakeDamageFromEnemy(InHitInfo);
}

void AThrCharacter::TakeDamageFromEnemy(const FHitResult& InHitInfo)
{
	if (!HasAuthority())
		return;

	auto* OwningPlayerState = GetPlayerState<AThrPlayerState>();
	if (!OwningPlayerState)
		return;

	const FName ClosestBoneName = GetMesh()->FindClosestBone(InHitInfo.ImpactPoint);
	const auto& ClosestBoneStr = ClosestBoneName.ToString();
	const auto& ThoraxBoneStr = (GetMesh()->GetBoneIndex(TEXT("spine_04")) == INDEX_NONE ? TEXT("spine_02") : TEXT("spine_04"));

	EThrActionType Action = (GetMesh()->BoneIsChildOf(ClosestBoneName, ThoraxBoneStr) ? DeathThorax : HitAbdomen);
	float RateDecreasedSP = (Action == DeathThorax ? 4.f : 2.f);

	if (GetMesh()->BoneIsChildOf(ClosestBoneName, TEXT("neck_01")) || ClosestBoneStr.Contains(TEXT("neck_01")))
	{
		const auto& HitLocation = InHitInfo.ImpactPoint;
		const auto& HeadLocation = GetMesh()->GetBoneLocation(TEXT("head"));
		const auto& HeadToHitVector = HitLocation - HeadLocation;

		const double CosTheta = UKismetMathLibrary::Dot_VectorVector(GetActorRightVector().GetSafeNormal2D(), HeadToHitVector.GetSafeNormal2D());
		const double Angle = UKismetMathLibrary::Acos(CosTheta) * 180.f / UKismetMathLibrary::GetPI();

		Action = (Angle > 120.f ? DeathHeadLeft : (Angle < 60.f ? DeathHeadRight : DeathHeadCenter));
		RateDecreasedSP = 4.f;
	}
	else if (GetMesh()->BoneIsChildOf(ClosestBoneName, TEXT("clavicle_l")) || ClosestBoneStr.Contains(TEXT("hand_l")))
	{
		Action = HitArmsLeft;
		RateDecreasedSP = 1.f;
	}
	else if (GetMesh()->BoneIsChildOf(ClosestBoneName, TEXT("clavicle_r")) || ClosestBoneStr.Contains(TEXT("hand_r")))
	{
		Action = HitArmsRight;
		RateDecreasedSP = 1.f;
	}
	else if (GetMesh()->BoneIsChildOf(ClosestBoneName, TEXT("thigh_l")) || ClosestBoneStr.Contains(TEXT("thigh_l")) || ClosestBoneStr.Contains(TEXT("foot_l")))
	{
		Action = HitLegsLeft;
		RateDecreasedSP = 1.f;
	}
	else if (GetMesh()->BoneIsChildOf(ClosestBoneName, TEXT("thigh_r")) || ClosestBoneStr.Contains(TEXT("thigh_r")) || ClosestBoneStr.Contains(TEXT("foot_r")))
	{
		Action = HitLegsRight;
		RateDecreasedSP = 1.f;
	}

	const float DecreaseSP = OwningPlayerState->MaxSP - 25.f * RateDecreasedSP;
	OwningPlayerState->MaxSP = FMath::Max(DecreaseSP, 0.f);
	OwningPlayerState->CurSP = FMath::Min(OwningPlayerState->CurSP, OwningPlayerState->MaxSP);

	OwningPlayerState->bDead = (RateDecreasedSP == 4.f);
	OwningPlayerState->LastTakeDamageTime = FDateTime::UtcNow();

	if (!OwningPlayerState->MaxSP)
	{
		if (auto* Player = Cast<AThrPlayerController>(Controller))
			Player->ServerTryToSetBattleState(ThrMatchState::StopBattle);
	}

	ServerPlayAction(Action, true);

	MulticastBloodEffect(InHitInfo.ImpactPoint);
	MulticastBloodDecal(InHitInfo);
}

void AThrCharacter::SetInput(const bool bInit)
{
	if (auto* Player = Cast<AThrPlayerController>(Controller))
	{
		if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player->GetLocalPlayer()))
			bInit ? InputSubsystem->AddMappingContext(InputMappingContext, 0) : InputSubsystem->RemoveMappingContext(InputMappingContext);
	}
}

void AThrCharacter::MulticastSetCamera_Implementation()
{
	if (CAMERA)
		CAMERA->AddCharacter(this, bLeftSide);
}

void AThrCharacter::Move(const FInputActionValue& InValue)
{
	const auto& MovementVector = InValue.Get<FVector2D>() * (bLeftSide ? 1.f : -1.f);

	if (!Controller || CanPlayAction(MovementVector))
		return;

	const auto& Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const auto& ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const auto& RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AThrCharacter::SetAction(const EThrActionType InAction, const bool bInStarted)
{
	CurAction = (bInStarted ? InAction : (CurAction == InAction ? NoneAction : CurAction));
}

const bool AThrCharacter::CanPlayAction(const FVector2D& InMovementVector)
{
	if (bPlayAction)
		return true;

	if (InMovementVector.Y < 0.f)
		return false;

	EThrActionType Action = NoneAction;

	switch (CurAction)
	{
	case AttackTop:
		Action = (!InMovementVector.X ? CurAction : (InMovementVector.X > 0.f ? AttackTopRight : AttackTopLeft));
		break;
	default:
		return false;
	}

	bPlayAction = true;
	ServerPlayAction(Action);

	return true;
}

bool AThrCharacter::ServerPlayAction_Validate(const EThrActionType InAction, const bool bInForce)
{
	RPC_VALIDATE(InAction != NoneAction);
	return true;
}

void AThrCharacter::ServerPlayAction_Implementation(const EThrActionType InAction, const bool bInForce)
{
	auto* OwningPlayerState = GetPlayerState<AThrPlayerState>();
	if (!OwningPlayerState || (!bInForce && OwningPlayerState->CurSP < 20.f))
		return;

	if (!bInForce)
		OwningPlayerState->CurSP -= 20.f;

	MulticastPlayAction(InAction);
	MulticastUpdateSP(OwningPlayerState->MaxSP, OwningPlayerState->CurSP);
}

void AThrCharacter::MulticastPlayAction_Implementation(const EThrActionType InAction) const
{
	OnPlayAction.ExecuteIfBound(InAction);
}

void AThrCharacter::MulticastUpdateSP_Implementation(const float InMaxSP, const float InCurSP)
{
	OnUpdateSP.ExecuteIfBound(bLeftSide, InMaxSP, InCurSP);
}

void AThrCharacter::ClientSparkEffect_Implementation(UPrimitiveComponent* InHitComponent, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, FVector InNormalImpulse, const FHitResult& InHitInfo)
{
	if (SparkEffect && InOtherComp && InOtherComp->GetFName() == TEXT("Weapon"))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SparkEffect, InHitInfo.ImpactPoint, FRotator::ZeroRotator, FVector(0.1f));
}

void AThrCharacter::MulticastBloodEffect_Implementation(const FVector& InImpactPoint)
{
	if (BloodEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BloodEffect, InImpactPoint, FRotator::ZeroRotator, FVector(0.01f));
}

void AThrCharacter::MulticastBloodDecal_Implementation(const FHitResult& InHitInfo)
{
	if (BloodDecal)
	{
		const FName ClosestBoneName = GetMesh()->FindClosestBone(InHitInfo.ImpactPoint);
		BloodDecal->SpawnDecal(InHitInfo.ImpactPoint, InHitInfo.ImpactNormal.ToOrientationQuat(), ClosestBoneName, 30.f);
	}
}

bool AThrCharacter::ServerEndBattle_Validate(const int32 InBattleResult)
{
	RPC_VALIDATE(InBattleResult);
	return true;
}

void AThrCharacter::ServerEndBattle_Implementation(const int32 InBattleResult)
{
	if (auto* OwningPlayerState = GetPlayerState<AThrPlayerState>())
	{
		if (!OwningPlayerState->bDead)
			ServerPlayAction((OwningPlayerState->BattleResult > 0.f ? Victory : Surrender), true);
	}
}
