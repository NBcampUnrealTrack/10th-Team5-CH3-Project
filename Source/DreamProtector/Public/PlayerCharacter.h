#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"


class UProjectilePoolComponent;
class USpringArmComponent;
class UCameraComponent;
class AWeaponBase;
class UInventoryComponent;
class UAnimMontage;
class UInputAction;
class AWindupBomb;
class ABarricade;
//딜리게이트 2개의 값을 전달하겠다는 매크로
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FonManaChanged,
	int32, CurrentMana,
	int32, MaxMana
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,
	float, CurrentHealth,
	float, MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionChanged,
	bool,
	bCanInteract,
	FText,
	InteractionText
);

UENUM(BlueprintType)
enum class EPlayControlMode : uint8
{
	ThirdPerson UMETA(DisplayName = "Third Person"),
	FirstPerson UMETA(DisplayName = "First Person"),
	Shoulder UMETA(DisplayName = "Shoulder / Aim")
};
UCLASS()
class DREAMPROTECTOR_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	APlayerCharacter();
	//블루프린트에서 이벤트를 연결할수있게해줌
	//위 매크로에서 만든 딜리게이트타입의 변수
	//마나 변경 알림 
	UPROPERTY(BlueprintAssignable, Category = "Mana")
	FonManaChanged OnManaChanged;
	//체력 변경 알림
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;
	//플레이어 사망 알림
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerDied OnPlayerDied;
	//상호작용 가능 여부 알림
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionChanged OnInteractionChanged;


protected:
	
	virtual void BeginPlay() override;
	void Die();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USceneComponent* CastPoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHP = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHP;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mana")
	int32 MaxMana = 30;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mana")
	int32 CurrentMana;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mana")
	bool bIsReloading = false;

	// 연사 시작까지 기다리는 타이머
	FTimerHandle AutoFireStartTimerHandle;
	// 실제 연사 반복 타이머
	FTimerHandle AutoFireTimerHandle;
	// 단발 타이머
	FTimerHandle SingleFireCooldownTimer;

	// 몇 초 이상 누르면 연사로 판단할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AutoFireHoldTime = 0.25f;
	// 연사 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackInterval = 0.2f;
	// 현재 연사 중인지
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	bool bIsAutoFiring = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float SingleFireCooldown = 0.3f;
	// 별사탕 공격력 버프 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	float AttackDamageMultiplier = 1.0f;
	// 별사탕 버프 종료 타이머
	FTimerHandle StarCandyTimerHandle;
	// 태엽 신발 사용 전 이동속도를 저장
	float OriginalWalkSpeed = 0.0f;

	// 태엽 신발 버프 종료용 타이머
	FTimerHandle GearShoesTimerHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	bool bIsAttacking = false;
	bool bCanAttack = true;

	// 현재 상호작용 애니메이션을 재생 중인지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsInteracting = false;


 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeaponBase* CurrentWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* InventoryComponent;

	//애니메이션 몽타주 장전 및 단발 연발
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AutoFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ReloadMontage;

	//플레이어 피격 시 재생 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage>HitReactMontage;

	// 상호작용 시 재생할 전신 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> InteractMontage;

	// 실제로 상호작용할 Actor를 임시로 저장
	UPROPERTY()
	TObjectPtr<AActor> PendingInteractActor;

	//현재 모드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	EPlayControlMode CurrentControlMode = EPlayControlMode::ThirdPerson;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ChangeControlModeAction;
	// 월드에 생성할 태엽 폭탄 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AWindupBomb> WindupBombClass;
	// 실제 월드에 설치할 바리케이드 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<ABarricade> BarricadeClass;

	// 설치형 아이템을 플레이어 앞에 생성할 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float ItemSpawnDistance = 150.0f;


public:	
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FVector2D& MovementVector);
	void StartSprint();
	void StopSprint();


	float GetMaxHP()const;
	float GetCurrentHP()const;
	float GetMaxMana()const;
	float GetCurrentMana()const;
	bool ConsumeMana();
	void ReloadMana();
	UFUNCTION(BlueprintCallable, Category = "Reload")
	void FinishReload();
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartAttack();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopAttack();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartAutoFire();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AutoAttack();
	UFUNCTION(BlueprintImplementableEvent, Category = "Sound")
	void PlayReloadSound();
	UFUNCTION(BlueprintImplementableEvent, Category = "Sound")
	void PlayAttackSound();
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void FireCurrentWeapon();
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void EndAttackAnimation();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	USceneComponent* GetCastPoint() const
	{
		return CastPoint;
	}

	UProjectilePoolComponent* GetProjectilePoolComponent() const
	{
		return ProjectilePoolComponent;
	}

	// 상호작용 애니메이션 종료 후 플레이어의 이동을 다시 허용한다.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteraction();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectilePoolComponent* ProjectilePoolComponent;

	//시점에 관한 함수
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ChangeControlMode();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyControlMode(EPlayControlMode NewControlMode);

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool UseItem(FName ItemKey);
	float GetAttackDamageMultiplier() const;
	// 몬스터 공격 등으로 플레이어가 데미지를 받을 때 호출
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamageFromEnemy(float DamageAmount);

	// Anim Notify에서 호출하여 실제 상호작용을 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ExecuteInteraction();

	// 피격 애니메이션 재생
	void PlayHitReaction();

	void CheckInteractable();
};
