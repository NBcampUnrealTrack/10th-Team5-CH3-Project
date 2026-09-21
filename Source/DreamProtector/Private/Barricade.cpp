#include "Barricade.h"
#include "Components/StaticMeshComponent.h"

ABarricade::ABarricade()
{
    PrimaryActorTick.bCanEverTick = false;

    BarricadeMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarricadeMesh"));

    SetRootComponent(BarricadeMesh);

    // 몬스터가 통과하지 못하도록 충돌 활성화
    BarricadeMesh->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics
    );

    BarricadeMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void ABarricade::BeginPlay()
{
    Super::BeginPlay();

    // Duration초가 지나면 자동 삭제
    SetLifeSpan(Duration);
}