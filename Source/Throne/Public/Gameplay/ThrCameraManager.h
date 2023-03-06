#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ThrCameraManager.generated.h"

#define CAMERA AThrCameraManager::Ptr

UCLASS()
class THRONE_API AThrCameraManager : public APawn
{
	GENERATED_BODY()

public:
	static AThrCameraManager* Ptr;

	AThrCameraManager();

	virtual void Tick(float InDeltaTime) override;
	
	void AddCharacter(const TWeakObjectPtr<AActor>& InCharacter, const bool bInLeftSide);

private:
	void CalculateValue();
	
	void SetCameraPosition();
	void SetP1RelativeValue();
	void RotateDefaultScene();

	UPROPERTY()
	class USceneComponent* RootScene;

	UPROPERTY()
	class USpringArmComponent* SpringArm;

	UPROPERTY()
	class UCameraComponent* Camera;

	TArray<TWeakObjectPtr<AActor>> Characters;

	const float RotateForce = 200.f;

	const float Height = 20.f;
	const float SpringArmBaseDistance = 150.f;
	const float SpringArmExtraDistance = 15.f;
	const float SpringArmMaxRotation = 5.f;
	const float SpringArmMinRotation = 3.f;

	float ForwardAxisValue;
	float GlobalDistanceFactor;
	float P1ToRootInnerAngle;
};
