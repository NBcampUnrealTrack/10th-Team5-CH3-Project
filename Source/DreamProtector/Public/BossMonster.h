#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossMonster.generated.h"

class UBehaviorTree;
class UAnimMontage;

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1     UMETA(DisplayName = "Phase 1"),
	Phase2     UMETA(DisplayName = "Phase 2"),
	Phase3     UMETA(DisplayName = "Phase 3")
};

UENUM(BlueprintType)
enum class EBossIntroState : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	Lunging    UMETA(DisplayName = "Lunging"),
	Pausing    UMETA(DisplayName = "Pausing"),
	Rising     UMETA(DisplayName = "Rising"),
	Done       UMETA(DisplayName = "Done")
};

// UI 에게 HP 동기화용으로 브로드캐스트할 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossHealthChanged, float, CurrentHealth, float, MaxHealth);

// 페이즈 전환 시 UI / 이펙트쪽 같이 반응할 수 있는 브로드캐스트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, EBossPhase, NewPhase);

// 보스 사망 시 (사망 몽타주 재생 후) 브로드 캐스트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDied);

UCLASS()
class DREAMPROTECTOR_API ABossMonster : public ACharacter
{
	GENERATED_BODY()

public:

	ABossMonster();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	//----------- 체력, 페이즈

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Stats")
	float MaxHealth = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Phase")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

	// 전환 중인 다음 페이즈 (몽타주가 끝나면 적용)
	EBossPhase PendingPhase = EBossPhase::Phase1;

	// 페이즈 체력 비율 전환, 필요 시 에이터에서 조정 가능
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase2Threshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase3Threshold = 0.2f;

	//----------- 상태

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	bool bIsInvincible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	bool bIsDead = false;

	//----------- 이동
	// 평소 위치, 근접 공격 후 돌아올 좌표, Begin play 시점의 위치로 자동 세팅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Movement")
	FVector AnchorLocation;

	// 등장 씬에서 돌진할 목표 지점, 레벨의 "BossLungeTarget" 태그 액터 위치로 Beginplay에서 자동 세팅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Movement")
	FVector LungeTargetLocation;

	// 근접 공격 등으로 앵커를 벗어났다가 되돌아갈 때 호출 (Flying 모드라 집정 이동 필요)
	UFUNCTION(BlueprintCallable, Category = "Boss|Movement")
	FVector GetAnchorLocation() const;

	//----------- 등장 씬

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Intro")
	EBossIntroState CurrentIntroState = EBossIntroState::NotStarted;

	// 등장 씬이 끝나야 true. BT는 값이 true가 되기 전까지 대기 - 블랙보드에서 체크
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Intro")
	bool bIntroFinished = false;

	// 돌진/상승 이동 속도 (초당 유닛)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Intro")
	float IntroMoveSpeed = 600.0f;

	// 목표 지점 도달로 간주할 오차 범위
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Intro")
	float IntroArrivalTolerance = 10.0f;

	// 돌진 도착 후 상승 시작 전까지 대기 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Intro")
	float IntroPauseDuration = 1.5f;

	// 대기 끝나고 상승 단계 시작 (타이머 콜백용)
	void BeginRisingPhase();

	// 보스가 커지는 데 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro")
	float GrowDuration = 2.0f;

	// Roar와 상승이 시작된 후, 커지기까지 기다릴 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro")
	float GrowDelay = 0.6f;

	// 최종 Mesh 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro")
	float TargetMeshScale = 8.0f;

	// 성장 진행 상태
	bool bIsGrowing = false;
	float GrowElapsedTime = 0.0f;

	// 성장 시작 당시의 크기
	FVector GrowStartScale = FVector::OneVector;

	// 성장과 상승을 시작하는 위치
	FVector GrowStartLocation = FVector::ZeroVector;

	FTimerHandle IntroPauseTimerHandle;

	// 트리거 볼륨에서 플레이어 오버랩 시 호출
	UFUNCTION(BlueprintCallable, Category = "Boss|Intro")
	void StartIntroSequence();

	void OnLungeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 돌진 시 재생 (의자에서 앞으로 살짝 튀어나가는 모션)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* LungeMontage;

	// [테스트용] 트리거 볼륨 완성 전, BeginPlay 몇 초 후 자동으로 등장 씬 실행
	FTimerHandle TestIntroTimerHandle;

	// 상승하며 포효하는 모션
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* RoarMontage;

	// 등장 씬 종료 후, 매 프레임 플레이어 쪽으로 회전하는 속도 (도/초)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Movement")
	float FaceTargetRotationSpeed = 180.f;

	// 사망 시 장판 이펙트를 재생할 위치. 레벨의 "BossDeathGround" 태그 액터 위치로 BeginPlay에서 자동 세팅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Movement")
	FVector DeathGroundLocation;

	//----------- 애니메이션, 이펙트

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* DeathMontage;

	// 사망 시 발밑에 재생할 이펙트 (예: P_ky_magicCircle1)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	class UParticleSystem* DeathGroundEffect;

	// 페이즈 전환 시 재생할 모션 (Stylized Death 에셋에서 포효, 리액션 등 매핑)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* PhaseTransitionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	class UParticleSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	class USoundBase* HitSound;

	//------------ 근거리 공격

	// 근접 공격이 발동하는 거리
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float MeleeAttackRange = 300.f;

	// 근접 공격 시전 애니메이션 (낫 휘두르기)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	UAnimMontage* MeleeAttackMontage;

	// 애니메이션 시작 후, 실제 타격 판정이 일어나기까지의 시간
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float MeleeAttackCastDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float MeleeDamage = 15.f;

	// 넉백 세기
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float MeleeKnockbackForce = 800.f;

	// 근접 공격 시전 (BT 태스크에서 호출). 범위 밖이면 false 반환
	UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
	bool TryStartMeleeAttack();

	// 근접 공격 모션이 이미 진행 중인지
	bool bIsMeleeAttacking = false;

	// 실제 타격 판정 (타이머 콜백용)
	void ApplyMeleeHit();

	FTimerHandle MeleeAttackTimerHandle;

	FTimerHandle MeleeAttackEndTimerHandle;
	void EndMeleeAttack();

	// 낫 휘두를 때 재생할 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	USoundBase* MeleeSwingSound;

	FTimerHandle MeleeSwingSoundTimerHandle;

	// 타격 성공 시 재생할 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	USoundBase* MeleeHitSound;

	void PlayMeleeSwingSound();

	//------------ 원거리 공격

	// 원거리 공격용 발사체 클래스 (BP_ky_thunderBall 등을 BP_BossMonster에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	TSubclassOf<AActor> RangedProjectileClass;

	// 원거리 공격 시전 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	UAnimMontage* RangedAttackMontage;

	// 애니메이션 시작 후, 실제 발사체가 나가기까지의 시간 (시전 동작에 맞춰 조절)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float RangedAttackCastDelay = 1.2f;

	// 보스 몸통 중심에서 앞으로/위로 얼마나 떨어진 지점에서 발사할지 (자기 자신과의 충돌 방지 + 손/낫 위치 보정용)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float RangedSpawnForwardOffset = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float RangedSpawnUpOffset = 1400.f;

	// 원거리 공격 시전 (BT에서 호출할 함수) - 애니메이션 재생 후 지연 발사
	UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
	void FireRangedAttack();

	// 실제 발사체를 스폰하는 내부 함수 (타이머 콜백용)
	void SpawnRangedProjectile();

	FTimerHandle RangedAttackTimerHandle;

	// 원거리 공격 시전 시 재생할 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	USoundBase* RangedAttackCastSound;

	FTimerHandle RangedAttackSoundTimerHandle;

	//------------ 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossHealthChanged OnBossHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossDied OnBossDied;
	//----------- 2페이즈

	// 2페이즈 진입 연출 전용 (비워두면 연출 없이 바로 전환)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* Phase2TransitionMontage;

	//----------- 3페이즈

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	class UStaticMesh* LaserBeamMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserLength = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserThickness = 60.f;

	// 빔 판정의 세로 두께
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserHeight = 250.f;

	// 초당 회전 속도(도)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserRotationSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserDamagePerTick = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserDamageInterval = 0.5f;

	// 판정 박스를 눈에 보이는 빔보다 얼마나 더 굵게 할지
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase3")
	float LaserHitboxScale = 1.5f;

	// 3페이즈 진입 연출 전용 (Staff_Attack_B 등)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* Phase3TransitionMontage;

	bool bLaserActive = false;

	// 레이저 빔이 도는 높이. 레벨의 "BossLaserGround" 태그 액터 Z값으로 BeginPlay에서 세팅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Phase3")
	FVector LaserGroundLocation;

	UPROPERTY()
	class USceneComponent* LaserPivot;

	UPROPERTY()
	TArray<class UBoxComponent*> LaserBeams;

	FTimerHandle LaserDamageTimerHandle;

protected:

	// 체력 변화 후 페이즈 전화 여부 체크 -> 필요 시 StartPhaseTransition 호출
	void CheckPhaseTransition();

	// 무적 + 모션(있다면) 재생, 끝나면 CurrentPhase 갱신 + 블랙보드에 반영
	void StartPhaseTransition(EBossPhase NewPhase);

	UFUNCTION()
	void OnPhaseTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 사망 처리 - 무적, 피격 비활성화 -> AI 정지 -> 사망 몽타주 재생 -> 완료 후 제거
	void HandleDeath();

	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 블랙보드의 CurrentPhase(정수) 키 갱신
	void UpdatePhaseInBlackboard();

	void BeginGrowing();

	void PlayRangedAttackSound();

	// 3페이즈 레이저
	void StartLaserPhase();

	void StopLaserPhase();

	void ApplyLaserDamageTick();
};
