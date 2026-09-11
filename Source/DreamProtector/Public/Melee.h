#pragma once

#include "CoreMinimal.h"
#include "BaseMonster.h"
#include "Components/SphereComponent.h"
#include "Melee.generated.h"

UCLASS()
class DREAMPROTECTOR_API AMelee : public ABaseMonster
{
	GENERATED_BODY()
	
public:

	AMelee();

	// BT Decorator가 타겟이 근접 공격 범위 안 인지 체크 함수
	UFUNCTION(BlueprintCallable, Category = "Melee")
	bool IsTargetInMeleeRange() const;
	// BT Decorator가 쿨타임이 지나 공격 가능한지 체크
	UFUNCTION(BlueprintCallable, Category = "Melee")
	bool CanAttack() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee")
	TObjectPtr<USphereComponent> AttackCollision;

	virtual void Attack_Implementation() override;

	// 공격 후 쿨타임 계산용
	float LastAttackTime = -999.f;
};
