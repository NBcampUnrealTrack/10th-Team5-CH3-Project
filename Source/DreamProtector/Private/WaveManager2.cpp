#include "WaveManager2.h"
#include "SpawnVolume.h"
#include "BaseMonster.h"
#include "Elevator.h"
#include "ElevatorKey.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AWaveManager2::AWaveManager2()
{
	PrimaryActorTick.bCanEverTick = true;

	BGMComponent =
		CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));

	BGMComponent->SetAutoActivate(false);
	BGMComponent->bAllowSpatialization = false;
	BGMComponent->SetUISound(false);
	BGMComponent->bAutoDestroy = false;
	BGMComponent->SetVolumeMultiplier(0.5f);
}

void AWaveManager2::BeginPlay()
{
	Super::BeginPlay();

	ChangeBGM(BattleBGM);

	// Stage2 시작과 동시에 2-1 전투 시작
	StartPhase1();
}

void AWaveManager2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 2-1은 레벨에 미리 배치된 몬스터를 처치하는 구간
void AWaveManager2::StartPhase1()
{
	bPhase1Started = true;
	bPhase2Started = false;

	ChangeBGM(BattleBGM);


	TArray<AActor*> Monsters;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		ABaseMonster::StaticClass(),
		Monsters
	);

	Phase1RemainingMonsterCount = Monsters.Num();
	Phase1TotalMonsterCount = Phase1RemainingMonsterCount;

	OnMonsterCountChanged.Broadcast(
		Phase1RemainingMonsterCount,
		Phase1TotalMonsterCount
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - 2-1 START / Monsters: %d ====="),
		Phase1RemainingMonsterCount
	);
}

void AWaveManager2::StartPreparation()
{
	ChangeBGM(PreparationBGM);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - PREPARATION START =====")
	);
}

// 2-2 전투 시작
void AWaveManager2::StartPhase2()
{
	bPhase2Started = true;
	ChangeBGM(BattleBGM);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - 2-2 COMBAT START =====")
	);

	// 2-2 몬스터를 한 번에 스폰
	SpawnPhase2Monsters();
}

void AWaveManager2::SpawnElevator()
{
	// 이미 엘리베이터가 생성됐다면 다시 생성하지 않음
	if (bElevatorSpawned)
	{
		return;
	}

	if (!ElevatorClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ElevatorClass is not set.")
		);
		return;
	}

	// 지정한 위치에 엘리베이터 생성
	SpawnedElevator =
		GetWorld()->SpawnActor<AElevator>(
			ElevatorClass,
			ElevatorSpawnTransform
		);

	if (SpawnedElevator)
	{
		bElevatorSpawned = true;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== STAGE 2 - ELEVATOR SPAWN =====")
		);
	}
}

void AWaveManager2::SpawnPhase2Monsters()
{
	if (Phase2SpawnData.Num() == 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Phase 2 Spawn Data is empty.")
		);
		return;
	}

	for (const FPhase2SpawnData& SpawnData : Phase2SpawnData)
	{
		

		if (!SpawnData.SpawnVolume || !SpawnData.MonsterClass)
		{
			continue;
		}

		for (int32 i = 0; i < SpawnData.SpawnCount; ++i)
		{
			FVector SpawnLocation =
				SpawnData.SpawnVolume->GetRandomSpawnLocation();

			ABaseMonster* SpawnedMonster =
				GetWorld()->SpawnActor<ABaseMonster>(
					SpawnData.MonsterClass,
					SpawnLocation,
					FRotator::ZeroRotator
				);

			if (SpawnedMonster)
			{
				// 2-2에서 스폰된 몬스터라고 표시
				SpawnedMonster->SetIsPhase2Monster(true);

				// 실제 스폰 성공한 몬스터만 카운트
				++Phase2TotalMonsterCount;
				++Phase2RemainingMonsterCount;

				UE_LOG(
					LogTemp,
					Warning,
					TEXT("Spawn SUCCESS - Index: %d"),
					CurrentPhase2SpawnIndex
				);
			}
		}
	}
	OnMonsterCountChanged.Broadcast(
		Phase2RemainingMonsterCount,
		Phase2TotalMonsterCount
	);
}

void AWaveManager2::SpawnNextPhase2Wave()
{
	// 다음 스폰 차수로 이동
	++CurrentPhase2SpawnIndex;

	// 3차 스폰까지 완료했으면 종료
	if (CurrentPhase2SpawnIndex >= 3)
	{
		GetWorldTimerManager().ClearTimer(Phase2SpawnTimerHandle);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== STAGE 2 - 2-2 ALL SPAWN COMPLETE =====")
		);

		return;
	}

	// 현재 차수의 몬스터 스폰
	SpawnPhase2Monsters();
}
void AWaveManager2::OnPhase1MonsterKilled()
{
	if (Phase1RemainingMonsterCount <= 0)
	{
		return;
	}

	--Phase1RemainingMonsterCount;

	OnMonsterCountChanged.Broadcast(
		Phase1RemainingMonsterCount,
		Phase1TotalMonsterCount
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Stage2 2-1 Remaining Monsters: %d"),
		Phase1RemainingMonsterCount
	);

	if (Phase1RemainingMonsterCount <= 0)
	{
		Phase1RemainingMonsterCount = 0;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== STAGE 2 - 2-1 COMPLETE =====")
		);

		// 문 개방
		OpenGate();

		// 바로 2-2 시작
		StartPhase2();
	}
}
void AWaveManager2::OnPhase2MonsterKilled()
{
	// 이미 클리어된 상태라면 중복 처리 방지
	if (Phase2RemainingMonsterCount <= 0)
	{
		return;
	}

	--Phase2RemainingMonsterCount;

	OnMonsterCountChanged.Broadcast(
		Phase2RemainingMonsterCount,
		Phase2TotalMonsterCount
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Stage2 2-2 Remaining Monsters: %d"),
		Phase2RemainingMonsterCount
	);

	// 2-2 몬스터를 전부 처치
	if (Phase2RemainingMonsterCount <= 0)
	{
		Phase2RemainingMonsterCount = 0;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== STAGE 2 - 2-2 COMBAT COMPLETE =====")
		);

		// 엘리베이터 생성
		SpawnElevator();

		// 엘리베이터 열쇠 생성
		SpawnBossKey();

		// 전투 종료 후 준비 BGM
		ChangeBGM(PreparationBGM);
	}
}

void AWaveManager2::SpawnBossKey()
{
	// 이미 BossKey가 생성되었다면 중복 생성하지 않는다.
	if (bBossKeySpawned)
	{
		return;
	}

	// 에디터에서 BP_BossKey 클래스가 지정되지 않았다면 생성하지 않는다.
	if (!BossKeyClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("BossKeyClass is not set.")
		);

		return;
	}

	// 월드에 BP_BossKey 생성
	AItemBase* SpawnedBossKey = GetWorld()->SpawnActor<AItemBase>(
		BossKeyClass,
		BossKeySpawnTransform
	);

	// 생성에 성공했는지 확인
	if (SpawnedBossKey)
	{
		bBossKeySpawned = true;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== BOSS KEY SPAWNED =====")
		);
	}
}

void AWaveManager2::OpenGate()
{
	if (!GateActor)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("GateActor is not set.")
		);
		return;
	}

	// BP_LeftDoor의 OpenGate 함수 호출
	GateActor->CallFunctionByNameWithArguments(
		TEXT("OpenGate"),
		*GLog,
		nullptr,
		true
	);

	if (GateActorRight)
	{
		GateActorRight->CallFunctionByNameWithArguments(
			TEXT("OpenGate"),
			*GLog,
			nullptr,
			true
		);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - GATE OPEN =====")
	);
}

void AWaveManager2::ChangeBGM(USoundBase* NewMusic)
{
	if (!BGMComponent)
	{
		return;
	}

	BGMComponent->Stop();
	BGMComponent->SetSound(NewMusic);

	if (NewMusic)
	{
		BGMComponent->Play();
	}
}

void AWaveManager2::NotifyPhase1Countdown()
{
    OnWaveCountdownStarted.Broadcast(1);
}

void AWaveManager2::NotifyPhase2Countdown()
{
    OnWaveCountdownStarted.Broadcast(2);
}

void AWaveManager2::SkipToBossReady()
{
    if (bSkippedToBossReady)
    {
        return;
    }

    bSkippedToBossReady = true;

    // 기존 웨이브와 준비시간의 예약을 모두 취소합니다.
    FTimerManager& Timers = GetWorldTimerManager();

    Timers.ClearTimer(Phase1TimerHandle);
    Timers.ClearTimer(Phase2TimerHandle);
    Timers.ClearTimer(PreparationTimerHandle);
    Timers.ClearTimer(FinalPreparationTimerHandle);
    Timers.ClearTimer(Phase1CountdownTimerHandle);
    Timers.ClearTimer(Phase2CountdownTimerHandle);
    Timers.ClearTimer(Phase2SpawnTimerHandle);
    Timers.ClearTimer(ElevatorTimerHandle);
    Timers.ClearTimer(GateTimerHandle);

    // 테스트용: 현재 맵의 일반 몬스터를 정리합니다.
    // BossMonster는 BaseMonster를 상속하지 않으므로 포함되지 않습니다.
    TArray<AActor*> Monsters;
    UGameplayStatics::GetAllActorsOfClass(
        this,
        ABaseMonster::StaticClass(),
        Monsters
    );

    for (AActor* Monster : Monsters)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }

    // 마지막 준비시간 이후의 진행 상태로 맞춥니다.
    StageElapsedTime = 260.0f;
    bPhase1Started = true;
    bPhase2Started = true;

    CurrentPhase2SpawnIndex = 3;
    Phase2RemainingMonsterCount = 0;

    // 통로와 다음 구간에 필요한 액터를 준비합니다.
    OpenGate();
    SpawnElevator();
		SpawnBossKey();

    // 보스 트리거에 들어가기 전까지 준비 음악을 재생합니다.
    ChangeBGM(PreparationBGM);
}

int32 AWaveManager2::GetCurrentMonsterCount() const
{
	if (bPhase2Started)
	{
		return Phase2RemainingMonsterCount;
	}

	return Phase1RemainingMonsterCount;
}

int32 AWaveManager2::GetCurrentTotalMonsterCount() const
{
	if (bPhase2Started)
	{
		return Phase2TotalMonsterCount;
	}

	return Phase1TotalMonsterCount;
}