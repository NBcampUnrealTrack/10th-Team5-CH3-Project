#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager2.generated.h"

// HUD에 카운트다운 시작과 웨이브 번호를 전달하는 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnStage2WaveCountdownStarted,
	int32, WaveNumber
);

class ASpawnVolume;
class ABaseMonster;
class AElevator;
class AElevatorKey;
class UAudioComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct FPhase2SpawnData
{
	GENERATED_BODY()

	// 어떤 SpawnVolume에서 스폰할지
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TObjectPtr<ASpawnVolume> SpawnVolume;

	// 스폰할 몬스터 종류
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TSubclassOf<ABaseMonster> MonsterClass;

	// 스폰할 몬스터 수
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 SpawnCount = 0;

	// 몇 번째 스폰에서 생성할지
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 SpawnIndex = 0;
};

UCLASS()
class DREAMPROTECTOR_API AWaveManager2 : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager2();
	// 2-2 몬스터가 죽었을 때 호출
	void OnPhase2MonsterKilled();

	// 블루프린트에서 연결할 수 있는 카운트다운 시작 알림
	UPROPERTY(BlueprintAssignable, Category = "Stage 2|UI")
	FOnStage2WaveCountdownStarted OnWaveCountdownStarted;

protected:
	virtual void BeginPlay() override;

	// Stage 2 시작 후 경과 시간(초)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage 2")
	float StageElapsedTime = 0.0f;

	// 각 시간 이벤트가 실행됐는지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage 2")
	bool bPhase1Started = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage 2")
	bool bPhase2Started = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage 2")
	bool bElevatorSpawned = false;

	// Stage 2에서 사용할 스폰볼륨 목록
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Spawn")
	TArray<TObjectPtr<ASpawnVolume>> SpawnVolumes;

	// 2-2 몬스터 스폰 설정
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Spawn")
	TArray<FPhase2SpawnData> Phase2SpawnData;

	// Stage2 시작 후 225초에 생성할 엘리베이터
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Elevator")
	TSubclassOf<AElevator> ElevatorClass;

	// 엘리베이터가 생성될 위치
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Elevator")
	FTransform ElevatorSpawnTransform;

	// 현재 2-2 스폰 차수
	int32 CurrentPhase2SpawnIndex = 0;

	void StartPhase1();
	void StartPhase2();
	void StartPreparation();

	// Stage 2 시간 이벤트를 관리하는 타이머
	FTimerHandle Phase1TimerHandle;
	FTimerHandle Phase2TimerHandle;
	FTimerHandle ElevatorTimerHandle;
	FTimerHandle GateTimerHandle;
	FTimerHandle PreparationTimerHandle;
	FTimerHandle FinalPreparationTimerHandle;\
	// 각 전투의 카운트다운 시작을 예약하는 타이머
	FTimerHandle Phase1CountdownTimerHandle;
	FTimerHandle Phase2CountdownTimerHandle;

	// HUD에 카운트다운 시작을 알리는 함수
	void NotifyPhase1Countdown();
	void NotifyPhase2Countdown();

	// 2-2 몬스터 스폰
	void SpawnPhase2Monsters();

	// 2-2 다음 스폰 처리
	void SpawnNextPhase2Wave();

	// 2-2 스폰 간격을 관리하는 타이머
	FTimerHandle Phase2SpawnTimerHandle;

	// 엘리베이터 생성 시점
	void SpawnElevator();

	// 2-2에서 스폰한 몬스터 수
	int32 Phase2TotalMonsterCount = 0;

	// 2-2에서 아직 살아있는 몬스터 수
	int32 Phase2RemainingMonsterCount = 0;

	// 2-2 클리어 후 생성할 엘리베이터 키
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Elevator Key")
	TSubclassOf<AElevatorKey> ElevatorKeyClass;

	// 엘리베이터 키가 생성될 위치
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Elevator Key")
	FTransform ElevatorKeySpawnTransform;

	// 엘리베이터 키가 이미 생성됐는지
	bool bElevatorKeySpawned = false;

	// 2-2 클리어 후 엘리베이터 키 생성
	void SpawnElevatorKey();

	// 90초 후 열릴 대문 블루프린트
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Gate")
	TObjectPtr<AActor> GateActor;

	// 오른쪽 대문
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Stage 2|Gate")
	TObjectPtr<AActor> GateActorRight;

	// 90초 후 대문 개방
	void OpenGate();

	// BGM을 재생하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<UAudioComponent> BGMComponent;

	// 튜토리얼 음악
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> TutorialBGM;

	// 웨이브 전투 음악
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BattleBGM;

	// 준비시간 음악
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> PreparationBGM;

	// 현재 음악을 멈추고 새 음악을 재생
	void ChangeBGM(USoundBase* NewMusic);
public:
	virtual void Tick(float DeltaTime) override;
};