#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WaveManager.generated.h"


// 하나의 웨이브에서 사용할 몬스터 정보를 저장하는 구조체
USTRUCT(BlueprintType)
struct FWaveMonsterData
{
    GENERATED_BODY()

    // 스폰할 몬스터 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> MonsterClass;

    // 해당 몬스터를 몇 마리 스폰할지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnCount = 0;
};

// 하나의 웨이브에 대한 전체 정보를 저장하는 구조체, 이 FWaveData는 DataTable에서 한 줄로 쓸 수 있는 구조체
USTRUCT(BlueprintType)
struct FWaveData : public FTableRowBase
{
    GENERATED_BODY()

    // 해당 웨이브에서 사용할 몬스터 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FWaveMonsterData> Monsters;

    // 해당 웨이브에서 사용할 스폰볼륨 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnVolumeCount = 0;

    // 해당 웨이브의 제한시간(초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaveTimeLimit = 60.0f;
};

UCLASS()
class DREAMPROTECTOR_API AWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveManager();
    // 몬스터가 처치됐을 때 호출
    void OnMonsterKilled();

protected:
    // 현재 진행 중인 웨이브 번호
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 CurrentWave = 1;

    // 전체 웨이브 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 MaxWave = 3;


    // 현재 웨이브에서 아직 처리해야 할 몬스터 수
    // 웨이브 시작 시 해당 웨이브의 몬스터 수를 넣어줌
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 CurrentMonsterCount = 0;

    // 웨이브에서 사용할 스폰볼륨 목록
    // 레벨에 배치된 스폰볼륨들을 웨이브매니저가 참조
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Spawn")
    //ASpawnVolume 객체들을 가리킬 수 있는 포인터 배열을 가지고 있겠다는 전방선언
    TArray<TObjectPtr<class ASpawnVolume>> SpawnVolumes;

    // 웨이브 정보를 담고 있는 데이터테이블/UDataTable=DataTable에셋을 담는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Data")
    TObjectPtr<UDataTable> WaveDataTable;

    // 현재 웨이브의 데이터를 데이터테이블에서 가져오기
    const FWaveData* GetCurrentWaveData() const;

    // 현재 웨이브의 몬스터 수를 계산하기
    void UpdateCurrentMonsterCount();

    // 현재 웨이브에서 처치한 몬스터 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 KilledMonsterCount = 0;

    // 현재 웨이브의 몬스터를 스폰하기.
    void SpawnCurrentWave();

    // 현재 웨이브의 제한시간 타이머 시작
    void StartWaveTimer();

    // 현재 웨이브의 제한시간이 끝났을 때 호출
    void OnWaveTimeExpired();

    // 현재 Wave 데이터 가져오기 → 몬스터 수 계산 → 몬스터 스폰
    virtual void BeginPlay() override;

    // 현재 웨이브 제한시간을 관리하는 타이머
    FTimerHandle WaveTimerHandle;
 
    // 튜토리얼 종료까지 대기하는 타이머
    FTimerHandle TutorialTimerHandle;

    // 다음 웨이브 시작까지 대기하는 타이머
    FTimerHandle NextWaveTimerHandle;

    // 다음 웨이브를 시작하는 함수
    void StartNextWave();

    // 튜토리얼이 끝났을 때 호출
    void StartFirstWave();


public:	
	virtual void Tick(float DeltaTime) override;

};
