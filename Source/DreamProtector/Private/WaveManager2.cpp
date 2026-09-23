#include "WaveManager2.h"
#include "SpawnVolume.h"
#include "BaseMonster.h"
#include "Elevator.h"
#include "ElevatorKey.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

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

    ChangeBGM(TutorialBGM);

    // 2-1 전투 시작 3초 전
    GetWorldTimerManager().SetTimer(
        Phase1CountdownTimerHandle,
        this,
        &AWaveManager2::NotifyPhase1Countdown,
        27.0f,
        false
    );

    // 2-2 전투 시작 3초 전
    GetWorldTimerManager().SetTimer(
        Phase2CountdownTimerHandle,
        this,
        &AWaveManager2::NotifyPhase2Countdown,
        132.0f,
        false
    );

    // 시작 후 30초: 2-1 전투 시작
    GetWorldTimerManager().SetTimer(
        Phase1TimerHandle,
        this,
        &AWaveManager2::StartPhase1,
        30.0f,
        false
    );

    // 시작 후 90초: 준비시간 시작
    GetWorldTimerManager().SetTimer(
        PreparationTimerHandle,
        this,
        &AWaveManager2::StartPreparation,
        90.0f,
        false
    );

    // 시작 후 135초: 2-2 전투 시작
    GetWorldTimerManager().SetTimer(
        Phase2TimerHandle,
        this,
        &AWaveManager2::StartPhase2,
        135.0f,
        false
    );

    // 225초: 두 번째 준비시간 시작
    GetWorldTimerManager().SetTimer(
        FinalPreparationTimerHandle,
        this,
        &AWaveManager2::StartPreparation,
        225.0f,
        false
    );

    // 270초: 준비시간 종료 후 엘리베이터 생성
    GetWorldTimerManager().SetTimer(
        ElevatorTimerHandle,
        this,
        &AWaveManager2::SpawnElevator,
        260.0f,
        false
    );

    // Stage 2 시작 후 90초에 대문 개방
    GetWorldTimerManager().SetTimer(
        GateTimerHandle,
        this,
        &AWaveManager2::OpenGate,
        90.0f,
        false
    );
}

void AWaveManager2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 2-1은 레벨에 미리 배치된 몬스터를 처치하는 구간
void AWaveManager2::StartPhase1()
{
	ChangeBGM(BattleBGM);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - 2-1 COMBAT START =====")
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
	ChangeBGM(BattleBGM);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== STAGE 2 - 2-2 COMBAT START =====")
	);

	// 1차 스폰
	SpawnPhase2Monsters();

	// 20초마다 다음 스폰 실행
	GetWorldTimerManager().SetTimer(
		Phase2SpawnTimerHandle,
		this,
		&AWaveManager2::SpawnNextPhase2Wave,
		20.0f,
		true
	);
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
		// 현재 스폰 차수에 해당하는 데이터만 처리
		if (SpawnData.SpawnIndex != CurrentPhase2SpawnIndex)
		{
			continue;
		}

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

void AWaveManager2::OnPhase2MonsterKilled()
{
	// 2-2에서 아직 살아있는 몬스터 수 감소
	--Phase2RemainingMonsterCount;

	// 3차 스폰(Index 2)까지 시작된 상태이고
	// 모든 2-2 몬스터가 죽었을 때만 2-2 종료
	if (Phase2RemainingMonsterCount <= 0 &&
		CurrentPhase2SpawnIndex >= 2)
	{
		Phase2RemainingMonsterCount = 0;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== STAGE 2 - 2-2 COMBAT COMPLETE =====")
		);

		SpawnElevator();
		// 2-2 전체 클리어 후 엘리베이터 키 생성
		SpawnElevatorKey();
	}
}

void AWaveManager2::SpawnElevatorKey()
{
	if (bElevatorKeySpawned)
	{
		return;
	}

	if (!ElevatorKeyClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ElevatorKeyClass is not set.")
		);
		return;
	}

	if (!SpawnedElevator)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("SpawnedElevator is invalid.")
		);
		return;
	}

	AElevatorKey* SpawnedKey =
		GetWorld()->SpawnActor<AElevatorKey>(
			ElevatorKeyClass,
			ElevatorKeySpawnTransform
		);

	if (SpawnedKey)
	{
		SpawnedKey->SetElevator(SpawnedElevator);

		bElevatorKeySpawned = true;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== ELEVATOR KEY SPAWN =====")
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
