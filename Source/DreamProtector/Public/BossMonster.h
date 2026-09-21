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

	FTimerHandle IntroPauseTimerHandle;

	//FVector IntroMoveStartLocation;
	//float IntroMoveElapsed = 0.0f;
	//float IntroMoveDuration = 1.0f;

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

	//----------- 애니메이션, 이펙트

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* DeathMontage;

	// 페이즈 전환 시 재생할 모션 (Stylized Death 에셋에서 포효, 리액션 등 매핑)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	UAnimMontage* PhaseTransitionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	class UParticleSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Anim")
	class USoundBase* HitSound;

	//------------ 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossHealthChanged OnBossHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossDied OnBossDied;

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

};
