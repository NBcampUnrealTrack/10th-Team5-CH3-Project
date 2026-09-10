#include "Melee.h"
#include "Kismet/GameplayStatics.h"

AMelee::AMelee()
{
   AttackCollision = CreateDefaultSubobject<USphereComponent>(TEXT("AttackCollision"));
   AttackCollision->SetupAttachment(RootComponent);
   AttackCollision->SetSphereRadius(100.0f);
   //이 컴포넌트는 충돌로 물리 반응은 하지 않고, 충돌/겹침 여부를 검색(Query)하는 용도로만 사용하겠다
   AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
   //모든 채널의 충돌 반응을 무시하도록 초기화
   AttackCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
   //Pawn 채널에 대해서만 Overlap 판정을 갖게 만든다.
   AttackCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
   AttackCollision->SetGenerateOverlapEvents(true);
}

void AMelee::Attack_Implementation()
{
    TArray<AActor*> OverlappingActors;
    AttackCollision->GetOverlappingActors(OverlappingActors);
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this)
        {
            continue;
        }
        //// 플레이어 액터에 "Player" 태그가 있어야 공격이 적용됩니다.
        if (!Actor->ActorHasTag(TEXT("Player")))
        {
            continue;
        }
        UE_LOG(LogTemp, Warning, TEXT("Melee Attack! Damage: %f"), AttackDamage);
        UGameplayStatics::ApplyDamage(Actor,AttackDamage,nullptr,this,UDamageType::StaticClass());
    }
}
