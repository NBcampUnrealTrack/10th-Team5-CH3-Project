#include "WaveManager.h"
#include "SpawnVolume.h"

AWaveManager::AWaveManager()
{
 	PrimaryActorTick.bCanEverTick = true;

}



// Called every frame
void AWaveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

const FWaveData* AWaveManager::GetCurrentWaveData() const
{
    // 현재 웨이브 번호를 데이터테이블의 Row 이름으로 만든다/ 예) CurrentWave = 1 → "Wave1"
    const FName RowName = FName(*FString::Printf(TEXT("Wave%d"), CurrentWave));

    // 데이터테이블에서 해당 Row를 찾아 반환
    return WaveDataTable->FindRow<FWaveData>(RowName, TEXT("GetCurrentWaveData"));
}

void AWaveManager::UpdateCurrentMonsterCount()
{
    // 현재 웨이브의 데이터를 가져오기
    const FWaveData* WaveData = GetCurrentWaveData();

    // 웨이브 데이터가 없으면 종료
    if (!WaveData)
    {
        return;
    }

    // 현재 웨이브의 몬스터 수를 0부터 다시 계산
    CurrentMonsterCount = 0;

    // 몬스터 종류별 스폰카운트를 모두 더하기
    for (const FWaveMonsterData& MonsterData : WaveData->Monsters)
    {
        CurrentMonsterCount += MonsterData.SpawnCount;
    }

    KilledMonsterCount = 0;

    // HUD에게 현재 몬스터 수 알림
    OnMonsterCountChanged.Broadcast(
        CurrentMonsterCount,
        CurrentMonsterCount
    );


}

void AWaveManager::SpawnCurrentWave()
{
    // 현재 웨이브 데이터 가져오기
    const FWaveData* WaveData = GetCurrentWaveData();

    // 웨이브 데이터 없으면 종료하는 if문
    if (!WaveData)
    {
        return;
    }

    //  새 웨이브 시작이므로 처치 수 초기화
    KilledMonsterCount = 0;
    //  현재 웨이브의 총 몬스터 수 다시 계산
    UpdateCurrentMonsterCount();

    // HUD에 새 웨이브 몬스터 수 전달
    OnMonsterCountChanged.Broadcast(
        CurrentMonsterCount,
        CurrentMonsterCount
    );

    // 현재 웨이브 몬스터 종류 하나씩 확인하기
    for (const FWaveMonsterData& MonsterData : WaveData->Monsters)
    {
        //몬스터 클래스가 지정되지 않았다면 스킵/.의 의미는 MonsterData 안에 있는 SpawnCount를 가져온다.
        if (!MonsterData.MonsterClass)
        {
            continue;
        }

        //설정된 몬스터 수만큼 반복 스폰시키기
        for (int32 i = 0; i < MonsterData.SpawnCount; ++i)
        {
            //사용할 스폰볼륨이 없다면 종료
            if (SpawnVolumes.Num() == 0)
            {
                return;
            }

            // 현재 웨이브에서 사용할 스폰볼륨에 몬스터 골고루 분배하기
            ASpawnVolume* SpawnVolume = SpawnVolumes[i % WaveData->SpawnVolumeCount];

            // 스폰볼륨이 없으면 해당 스폰 스킵
            if (!SpawnVolume)
            {
                continue;
            }

            // 스폰볼륨에서 랜덤한 스폰 위치 가져오기
            const FVector SpawnLocation = SpawnVolume->GetRandomSpawnLocation();

            // 몬스터 스폰하기
            GetWorld()->SpawnActor<AActor>(
                MonsterData.MonsterClass, SpawnLocation, FRotator::ZeroRotator
            );
        }

    }
}
void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("===== WaveManager BeginPlay ====="));

    UpdateCurrentMonsterCount();

    // 튜토리얼 30초 후 Wave 1 시작
    GetWorldTimerManager().SetTimer(
        TutorialTimerHandle,
        this,
        &AWaveManager::StartFirstWave,
        30.0f,
        false
    );

}

void AWaveManager::StartWaveTimer()
{
    // 현재 웨이브의 데이터 가져오기
    const FWaveData* WaveData = GetCurrentWaveData();

    // 웨이브 데이터가 없으면 종료
    if (!WaveData)
    {
        return;
    }

    // 현재 웨이브의 제한시간으로 타이머 시작
    GetWorldTimerManager().SetTimer(
        WaveTimerHandle,
        this,
        &AWaveManager::OnWaveTimeExpired,
        WaveData->WaveTimeLimit,
        false
    );
}

void AWaveManager::OnWaveTimeExpired()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("===== WAVE TIME EXPIRED ===== Wave: %d"),
        CurrentWave
    );

    // 45초 후 다음 웨이브 시작
    GetWorldTimerManager().SetTimer(
        NextWaveTimerHandle,
        this,
        &AWaveManager::StartNextWave,
        45.0f,
        false
    );
}

void AWaveManager::StartFirstWave()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("===== WAVE 1 START =====")
    );

    UpdateCurrentMonsterCount();
    // Wave 1 몬스터 스폰
    SpawnCurrentWave();

    // Wave 1 제한시간 시작
    StartWaveTimer();
}

void AWaveManager::StartNextWave()
{
    // 다음 웨이브 번호로 변경
    CurrentWave++;

    if (CurrentWave > MaxWave)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("===== WAVE %d START ====="),
        CurrentWave
    );
    UpdateCurrentMonsterCount();
    // 다음 웨이브 몬스터 스폰
    SpawnCurrentWave();

    // 다음 웨이브 제한시간 시작
    StartWaveTimer();
}

void AWaveManager::OnMonsterKilled()
{
    // 처치한 몬스터 수 증가
    KilledMonsterCount++;

    const int32 RemainingCount =
        FMath::Max(CurrentMonsterCount - KilledMonsterCount, 0);

    OnMonsterCountChanged.Broadcast(
        RemainingCount,
        CurrentMonsterCount
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Monster Killed: %d, Remaining: %d / %d"),
        KilledMonsterCount,
        RemainingCount,
        CurrentMonsterCount
    );
}