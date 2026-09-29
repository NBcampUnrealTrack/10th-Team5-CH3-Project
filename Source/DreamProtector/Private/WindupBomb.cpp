#include "WindupBomb.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseMonster.h"
#include "TimerManager.h"

AWindupBomb::AWindupBomb()
{
    PrimaryActorTick.bCanEverTick = false;

    BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
    SetRootComponent(BombMesh);

    ExplosionRange = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionRange"));
    ExplosionRange->SetupAttachment(BombMesh);

    ExplosionRange->SetSphereRadius(300.0f);
    ExplosionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ExplosionRange->SetCollisionResponseToAllChannels(ECR_Overlap);

    // 폭발 범위를 검사할 용도이므로 물리 충돌은 하지 않음
    ExplosionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWindupBomb::BeginPlay()
{
    Super::BeginPlay();

    // 설치되고 ExplosionDelay초 후 Explode 실행
    GetWorldTimerManager().SetTimer(
        ExplosionTimerHandle,
        this,
        &AWindupBomb::Explode,
        ExplosionDelay,
        false
    );
}

void AWindupBomb::Explode()
{
    UE_LOG(LogTemp, Warning, TEXT("Windup Bomb Exploded!"));

    // 폭발 범위 안에 들어와 있는 Actor들을 저장
    TArray<AActor*> OverlappingActors;

    ExplosionRange->GetOverlappingActors(
        OverlappingActors,
        ABaseMonster::StaticClass()
    );

    // 범위 안의 몬스터들에게 데미지 적용
    for (AActor* Actor : OverlappingActors)
    {
        ABaseMonster* Monster = Cast<ABaseMonster>(Actor);

        if (Monster)
        {
            UGameplayStatics::ApplyDamage(
                Monster,            // 데미지를 받을 Actor
                ExplosionDamage,    // 데미지 양
                nullptr,            // 공격 Controller
                this,               // 데미지 원인 Actor
                nullptr             // DamageType
            );

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Bomb Hit Monster: %s"),
                *Monster->GetName()
            );
        }
    }

    Destroy();
}