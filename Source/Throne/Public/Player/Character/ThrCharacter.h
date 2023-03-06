#pragma once

#include "../../Gamedata/ThrCharacterData.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ThrCharacter.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnUpdateSP, const bool, const float, const float);

UENUM(BlueprintType)
enum EThrInputType
{
	NoneInput,

	MoveInput,
	AttackTopInput,
};

UCLASS()
class THRONE_API AThrCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	static FOnUpdateSP OnUpdateSP;

	AThrCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float InDeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InPlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* InNewController) override;

	const TWeakObjectPtr<USkeletalMeshComponent> GetWeapon() const { return Weapon; }
	const FName GetCharacterName() const { return CharacterName; }
	void EndAction() { bPlayAction = false; }

	void ApplyDamageToEnemy(const FHitResult& InHitInfo);
	void TakeDamageFromEnemy(const FHitResult& InHitInfo);
	
	void SetInput(const bool bInit);

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetCamera();

	void Move(const FInputActionValue& InValue);

	void SetAction(const EThrActionType InAction, const bool bInStarted);

	const bool CanPlayAction(const FVector2D& InMovementVector);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayAction(const EThrActionType InAction, const bool bInForce = false);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAction(const EThrActionType InAction) const;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateSP(const float InMaxSP, const float InCurSP);
	
	UFUNCTION(Client, Reliable)
	void ClientSparkEffect(UPrimitiveComponent* InHitComponent, AActor* InOtherActor, UPrimitiveComponent* InOtherComp, FVector InNormalImpulse, const FHitResult& InHitInfo);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBloodEffect(const FVector& InImpactPoint);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBloodDecal(const FHitResult& InHitInfo);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndBattle(const int32 InBattleResult);
	
public:
	DECLARE_DELEGATE_OneParam(FOnPlayAction, const EThrActionType);
	FOnPlayAction OnPlayAction;

protected:
	UPROPERTY(EditDefaultsOnly)
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EThrInputType>, class UInputAction*> InputActions;
	
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* Weapon;
	
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* SparkEffect;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* BloodEffect;

private:
	UPROPERTY()
	class UThrSkinnedDecalComponent* BloodDecal;
	
	UPROPERTY()
	class UPhysicalAnimationComponent* MeshPhysicalAnim;

	UPROPERTY()
	class UPhysicalAnimationComponent* WeaponPhysicalAnim;
	
	UPROPERTY(EditDefaultsOnly)
	class UPhysicsConstraintComponent* WeaponPhysicsConstraint;

	UPROPERTY(Replicated)
	bool bLeftSide;

	UPROPERTY(Replicated)
	FName CharacterName;

	bool bPlayAction;
	TEnumAsByte<EThrActionType> CurAction = NoneAction;
};
