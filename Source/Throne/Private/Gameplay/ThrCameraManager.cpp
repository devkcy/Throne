#include "Gameplay/ThrCameraManager.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AThrCameraManager* AThrCameraManager::Ptr;

AThrCameraManager::AThrCameraManager()
	: RootScene(CreateDefaultSubobject<USceneComponent>(TEXT("RootScene")))
	, SpringArm(CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm")))
	, Camera(CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")))
{
 	PrimaryActorTick.bCanEverTick = true;

	if (RootScene)
	{
		RootComponent = RootScene;
		RootScene->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
	}

	if (SpringArm)
	{
		SpringArm->SetupAttachment(RootComponent);
		SpringArm->TargetArmLength = 450.f;
		SpringArm->bDoCollisionTest = false;
		SpringArm->bEnableCameraLag = true;
		SpringArm->CameraLagSpeed = 20.f;
	}

	if (Camera)
	{
		Camera->SetupAttachment(SpringArm);
		Camera->bUsePawnControlRotation = false;
	}

	Characters.Init(nullptr, 2);
}

void AThrCameraManager::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
	
	for (const auto& Character : Characters)
	{
		if (!Character.IsValid())
			return;
	}

	CalculateValue();
}

void AThrCameraManager::AddCharacter(const TWeakObjectPtr<AActor>& InCharacter, const bool bInLeftSide)
{
	if (Characters.Contains(InCharacter))
		return;

	const int32 IndexBySide = (bInLeftSide ? 0 : 1);
	Characters[IndexBySide] = InCharacter;
}

void AThrCameraManager::CalculateValue()
{
	SetCameraPosition();
	SetP1RelativeValue();
	RotateDefaultScene();
}

void AThrCameraManager::SetCameraPosition()
{
	if (Characters.Num() < 2)
		return;

	for (const auto& Character : Characters)
	{
		if (!Character.IsValid())
			return;
	}

	const auto& P1Location = Characters[0]->GetActorLocation();
	const auto& P2Location = Characters[1]->GetActorLocation();

	const float P1ToP2Distance = (P2Location - P1Location).Size() / 1200.f;
	GlobalDistanceFactor = UKismetMathLibrary::FClamp(P1ToP2Distance, 0.f, 1.f);

	if (RootScene)
	{
		const auto& DefaultLocation = (P1Location + P2Location) / 2.f + FVector(0.f, 0.f, Height);
		RootScene->SetWorldLocation(DefaultLocation);
	}

	if (SpringArm)
	{
		const auto& P1ToP2Vector = (P2Location - P1Location) * FVector(1.f, 1.f, 0.f);

		const float P1ToP2CenterLength = (P1ToP2Vector / 2.f).Size();
		const float SpringArmLength = UKismetMathLibrary::Lerp(SpringArmBaseDistance, (SpringArmBaseDistance + SpringArmExtraDistance), GlobalDistanceFactor);
		const float SpringArmPitch = UKismetMathLibrary::Lerp(SpringArmMaxRotation, SpringArmMinRotation, GlobalDistanceFactor) * -1.f;
		
		SpringArm->TargetArmLength = P1ToP2CenterLength + SpringArmLength;
		SpringArm->SetRelativeRotation(FRotator(SpringArmPitch, 0.f, 0.f));
	}
}

void AThrCameraManager::SetP1RelativeValue()
{
	if (Characters.Num() < 2)
		return;

	for (const auto& Character : Characters)
	{
		if (!Character.IsValid())
			return;
	}

	const auto& CameraLocation = Camera->GetComponentLocation();

	const auto& P1Location = Characters[0]->GetActorLocation();
	const auto& P2Location = Characters[1]->GetActorLocation();

	const float P1ToCameraDistance = (P1Location - CameraLocation).Size();
	const float P2ToCameraDistance = (P2Location - CameraLocation).Size();
	ForwardAxisValue = (P2ToCameraDistance > P1ToCameraDistance ? -1.f : 1.f);
	
	const auto& P1ToRootDirection = UKismetMathLibrary::GetDirectionUnitVector(P1Location, RootScene->GetComponentLocation());
	P1ToRootInnerAngle = UKismetMathLibrary::Dot_VectorVector(RootScene->GetRightVector(), P1ToRootDirection);
}

void AThrCameraManager::RotateDefaultScene()
{
	const float AbsInnerAngle = FMath::Abs(P1ToRootInnerAngle);
	if (!RootScene || AbsInnerAngle > 0.98f)
		return;

	const float Force = (0.98f - AbsInnerAngle) * RotateForce;
	const float YawForce = ForwardAxisValue * Force * UGameplayStatics::GetWorldDeltaSeconds(this);
	
	RootScene->AddWorldRotation(FRotator(0.f, YawForce, 0.f));
}
