#include "WaveManager.h"
#include "SpawnVolume.h"
#include "BaseMonster.h"
#include "Bed.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AWaveManager::AWaveManager()
{
 	PrimaryActorTick.bCanEverTick = true;

    BGMComponent =
        CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));

    // 음악을 지정하고 직접 Play할 때 재생
    BGMComponent->SetAutoActivate(false);

    // 위치나 거리에 영향을 받지 않는 배경음
    BGMComponent->bAllowSpatialization = false;

    // 게임이 일시정지되면 음악도 일시정지
    BGMComponent->SetUISound(false);

    // 음악 교체 시에도 컴포넌트를 계속 사용
    BGMComponent->bAutoDestroy = false;

    BGMComponent->SetVolumeMultiplier(0.5f);
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

            // 몬스터 스폰하기, 몬수터 이동속도 랜덤값 적용
            ABaseMonster* SpawnedMonster = GetWorld()->SpawnActor<ABaseMonster>(
                MonsterData.MonsterClass,
                SpawnLocation,
                FRotator::ZeroRotator
            );

            if (SpawnedMonster)
            {
                SpawnedMonster->SetMoveSpeed(FMath::FRandRange(200.0f, 500.0f));

                UE_LOG(LogTemp, Warning, TEXT("AFTER RANDOM Speed: %f"), SpawnedMonster->MoveSpeed);
            }
        }

    }
}
void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("===== WaveManager BeginPlay ====="));

    ChangeBGM(TutorialBGM);

    UpdateCurrentMonsterCount();

    // 튜토리얼 27초에 HUD의 3 → 2 → 1 연출 시작
    GetWorldTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &AWaveManager::NotifyFirstWaveCountdown,
        27.0f,
        false
    );

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

    // 현재 웨이브의 남은 악몽 수 계산
    const int32 RemainingCount =
        FMath::Max(CurrentMonsterCount - KilledMonsterCount, 0);

    // 남은 악몽 1마리당 침대 스트레스 10 증가
    if (Bed && RemainingCount > 0)
    {
        Bed->IncreaseStress(RemainingCount * 10);
    }

    // HUD의 남은 악몽 수를 0으로 표시
    OnMonsterCountChanged.Broadcast(
        0,
        0
    );

    // 마지막 웨이브라면 다음 준비시간과 카운트다운을 예약하지 않음
    if (CurrentWave >= MaxWave)
    {
        // 마지막 웨이브가 끝났다면 음악 정지
        ChangeBGM(nullptr);

        // 스테이지 클리어 처리가 필요하다면 이 분기에서 별도로 실행
        return;
    }

    ChangeBGM(PreparationBGM);

    // 준비시간 42초에 다음 웨이브의 3 → 2 → 1 연출 시작
    GetWorldTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &AWaveManager::NotifyNextWaveCountdown,
        42.0f,
        false
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

    ChangeBGM(BattleBGM);

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

    ChangeBGM(BattleBGM);

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

void AWaveManager::NotifyFirstWaveCountdown()
{
    // 현재 웨이브 번호를 HUD에 전달 (첫 웨이브는 1)
    OnWaveCountdownStarted.Broadcast(CurrentWave);
}

void AWaveManager::NotifyNextWaveCountdown()
{
    // 다음 웨이브가 있을 때만 카운트다운 알림
    if (CurrentWave < MaxWave)
    {
        // 실제 웨이브 번호 증가는 StartNextWave()에서 처리하므로 여기서는 다음 번호만 전달
        OnWaveCountdownStarted.Broadcast(CurrentWave + 1);
    }
}

void AWaveManager::ChangeBGM(USoundBase* NewMusic)
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