#pragma once

#include "CoreMinimal.h"
#include "BaseMonster.h"
#include "FlyingRanged.generated.h"

UCLASS()
class DREAMPROTECTOR_API AFlyingRanged : public ABaseMonster
{
	GENERATED_BODY()

public:
	// 생성자
	AFlyingRanged();

protected:
	// 초기 비행 높이 세팅 예정
	virtual void BeginPlay() override;

public:
	// 매 프레임마다 추척 / 높이 / 장애물 / 공격 판정
	virtual void Tick(float DeltaTime) override;

protected:
	// 원거리 공격 (데미지, 투사체)
	virtual void Attack_Implementation() override;

	// 캐릭터를 향해 수평 이동
	void MoveTowardsTarget(float DeltaTime);
	// 지정된 높이를 유지 (z 축 보정)
	void MaintainFlightHeight(float DeltaTime);
	// 진행 방향에 장애물 있는지 라인트레이스 체크
	void AvoidObstacle(float DeltaTime);
	// 타켓이 공격 범위에 들어왔는지 확인
	bool CheckObstacleAhead();
	// 장애물 감시 시 피해서 이동
	bool IsTargetInAttackRange() const;
	// 쿨타임이 지나 공격 가능한지 확인
	bool CanAttack() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	// 공격 사거리 
	float AttackRange = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	// 공격 후 다음 공격까지 걸리는 쿨타임
	float AttackCooldown = 2.0f;
	// 마지막 공격 (쿨타임 확인을 위함)
	float LastAttackTime = -999.9f;

	// 비행 유지 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight")
	float FlyHeight = 300.0f;
	
	// 장애물 감지 라인트레이스 길이 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight")
	float ObstacleCheckDistance = 200.0f;

	// 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect")
	// 공격 발사 시점 (아마 꼬리쪽)
	TObjectPtr<UParticleSystem> AttackEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect")
	// 이펙트를 붙일 소켓 이름 ( 스켈레톤에 맞게 수정 필요함)
	FName AttackEffectSocketName = "Mouth";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	// 공격 시 재생하는 사운드
	TObjectPtr<USoundBase> AttackSound;
};

