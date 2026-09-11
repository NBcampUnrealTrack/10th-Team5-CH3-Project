#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class AWeaponBase;
class UInventoryComponent;
//딜리게이트 2개의 값을 전달하겠다는 매크로
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FonManaChanged,
	int32, CurrentMana,
	int32, MaxMana
);
UCLASS()
class DREAMPROTECTOR_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	APlayerCharacter();
	//블루프린트에서 이벤트를 연결할수있게해줌
	//위 매크로에서 만든 딜리게이트타입의 변수
	UPROPERTY(BlueprintAssignable, Category = "Mana")
	FonManaChanged OnManaChanged;

protected:
	
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mana")
	float ReloadTime = 2.0f;
	FTimerHandle ReloadTimerHandle;

 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeaponBase* CurrentWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* InventoryComponent;


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
	void FinishReload();
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Attack();

};
