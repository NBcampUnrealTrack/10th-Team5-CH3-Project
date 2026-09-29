#include "WaveManager.h"
#include "SpawnVolume.h"
#include "BaseMonster.h"
#include "Bed.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/SpotLight.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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
    const FWaveData* WaveData = GetCurrentWaveData();
    if (!WaveData)
    {
        return;
    }

    WaveMonsters.Reset();
    CurrentMonsterCount = 0;
    KilledMonsterCount = 0;
    bCleaningUpWave = false;

    const int32 UsableVolumeCount = FMath::Min(
        WaveData->SpawnVolumeCount,
        SpawnVolumes.Num()
    );

    if (UsableVolumeCount <= 0)
    {
        OnMonsterCountChanged.Broadcast(0, 0);
        return;
    }

    for (const FWaveMonsterData& MonsterData : WaveData->Monsters)
    {
        if (!MonsterData.MonsterClass)
        {
            continue;
        }

        for (int32 i = 0; i < MonsterData.SpawnCount; ++i)
        {
            ASpawnVolume* SpawnVolume =
                SpawnVolumes[i % UsableVolumeCount];

            if (!IsValid(SpawnVolume))
            {
                continue;
            }

            const FVector SpawnLocation =
                SpawnVolume->GetRandomSpawnLocation();

            ABaseMonster* SpawnedMonster =
                GetWorld()->SpawnActor<ABaseMonster>(
                    MonsterData.MonsterClass,
                    SpawnLocation,
                    FRotator::ZeroRotator
                );

            if (!IsValid(SpawnedMonster))
            {
                continue;
            }

            WaveMonsters.Add(SpawnedMonster);
            ++CurrentMonsterCount;

            SpawnedMonster->OnDestroyed.AddDynamic(
                this,
                &AWaveManager::HandleWaveMonsterDestroyed
            );

            SpawnedMonster->SetMoveSpeed(
                FMath::FRandRange(200.0f, 500.0f)
            );
        }
    }

    OnMonsterCountChanged.Broadcast(
        CurrentMonsterCount,
        CurrentMonsterCount
    );
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("===== WaveManager BeginPlay ====="));

    ChangeBGM(TutorialBGM);
    ChangeWaveLightColor(PreparationLightColor);

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
    if (bCleaningUpWave)
    {
        return;
    }

    if (CurrentWave == 3 &&
        UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("Stage1"))
    {
        return;
    }
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("===== WAVE TIME EXPIRED ===== Wave: %d"),
        CurrentWave
    );

    // 일괄 정리 중에는 개별 삭제로 카운트를 줄이지 않음
    bCleaningUpWave = true;

    // 실제로 남아 있는 이번 웨이브 몬스터 수를 셈
    int32 RemainingCount = 0;

    for (const TObjectPtr<ABaseMonster>& Monster : WaveMonsters)
    {
        if (IsValid(Monster.Get()))
        {
            ++RemainingCount;
        }
    }

    // 남은 몬스터 한 마리당 스트레스 +10
    if (IsValid(Bed.Get()) && RemainingCount > 0)
    {
        Bed->IncreaseStress(RemainingCount * 10);
    }

    // 남은 몬스터를 실제로 삭제
    // Die를 호출하지 않으므로 정리 과정에서 아이템을 드롭하지 않음
    for (const TObjectPtr<ABaseMonster>& Monster : WaveMonsters)
    {
        if (IsValid(Monster.Get()))
        {
            Monster->Destroy();
        }
    }

    WaveMonsters.Reset();

    // 화면 표시뿐 아니라 내부 숫자도 정리
    CurrentMonsterCount = 0;
    KilledMonsterCount = 0;

    OnMonsterCountChanged.Broadcast(0, 0);

    // 마지막 웨이브라면 다음 준비시간과 카운트다운을 예약하지 않음
    if (CurrentWave >= MaxWave)
    {
        // 마지막 웨이브가 끝났다면 음악 정지
        ChangeBGM(nullptr);
        ChangeWaveLightColor(PreparationLightColor);

        // 스테이지 클리어 처리가 필요하다면 이 분기에서 별도로 실행
        return;
    }

    ChangeBGM(PreparationBGM);
    ChangeWaveLightColor(PreparationLightColor);

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
    ChangeWaveLightColor(BattleLightColor);

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
    ChangeWaveLightColor(BattleLightColor);

    // 다음 웨이브 몬스터 스폰
    SpawnCurrentWave();

    // 다음 웨이브 제한시간 시작
    StartWaveTimer();
}

void AWaveManager::OnMonsterKilled()
{
    if (bCleaningUpWave)
    {
        return;
    }
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

    if (CurrentWave == 3 && CurrentMonsterCount > 0 &&
        RemainingCount == 0 && WaveMonsters.IsEmpty() &&
        UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("Stage1"))
    {
        // 중복 이동을 막고 이전 웨이브에서 예약한 타이머와 음악을 정리합니다.
        bCleaningUpWave = true;
        GetWorldTimerManager().ClearTimer(WaveTimerHandle);
        GetWorldTimerManager().ClearTimer(TutorialTimerHandle);
        GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
        GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
        ChangeBGM(nullptr);
        UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/Stage2")));
    }
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

void AWaveManager::ChangeWaveLightColor(const FLinearColor& NewColor)
{
    // 에디터에서 조명을 연결하지 않았다면 아무 작업도 하지 않음
    if (!WaveSpotLight)
    {
        return;
    }

    ULightComponent* LightComponent = WaveSpotLight->GetLightComponent();

    if (LightComponent)
    {
        LightComponent->SetLightColor(NewColor, false);
    }
}

void AWaveManager::HandleWaveMonsterDestroyed(AActor* DestroyedActor)
{
    NotifyMonsterRemoved(Cast<ABaseMonster>(DestroyedActor));
}

void AWaveManager::NotifyMonsterRemoved(ABaseMonster* Monster)
{
    if (bCleaningUpWave || !Monster)
    {
        return;
    }

    // 목록에 있는 몬스터만 제거하고 카운트
    // 이미 사망 처리했다면 목록에 없으므로 중복 집계하지 않음
    const int32 RemovedCount = WaveMonsters.RemoveAll(
        [Monster](const TObjectPtr<ABaseMonster>& Entry)
        {
            return Entry.Get() == Monster;
        }
    );

    if (RemovedCount > 0)
    {
        OnMonsterKilled();
    }
}