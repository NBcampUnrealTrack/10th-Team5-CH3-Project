#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"



UCLASS()
class DREAMPROTECTOR_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자 
	ABaseMonster();    
	// 이동 속도, CharacterMovementComponent에 적용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float MoveSpeed = 200.0f;
	void SetMoveSpeed(float NewSpeed);


protected:
	
	virtual void BeginPlay() override;

public:	
	// 매 프레임 호출 - 자식 클래스에서 사거리 체크 등
	virtual void Tick(float DeltaTime) override; 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// 추적 / 공격할 대상 지정
	UFUNCTION(BlueprintCallable, Category = "Monster|Damage")
	void SetTarget(AActor* InTarget); 

	UFUNCTION(BlueprintCallable, Category = "Monster|Damage")
	AActor* GetTarget() const { return Target; }

	// 데미지 받을면 호출 -> HP 깎고 0이하면 뒤짐(Die 호출)
	UFUNCTION(BlueprintCallable, Category = "Monster|Damage")
	virtual void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Monster|Damage")
	void Attack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Monster|Damage")
	void Die();

protected:

	virtual void Attack_Implementation();
	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float MaxHealth = 50.0f;
	// 현제 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float CurrentHealth;                
	// 공격력, Attack 에 데미지 계산
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float AttackDamage = 10.0f;      
    // 추적 / 공격 대상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	TObjectPtr<AActor> Target;      

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float AttackInterval = 1.5f;
	// 사망 처리 (이동, 충돌, 연출, Destory 등)
	UFUNCTION()
	virtual void Die_Implementation();             
};
