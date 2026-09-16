#include "ProjectilePoolComponent.h"
#include "ProjectileBase.h"

UProjectilePoolComponent::UProjectilePoolComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UProjectilePoolComponent::BeginPlay()
{
  Super::BeginPlay();

  // ProjectileClass가 지정되지 않았다면 생성할 수 없으므로 종료
  if (!ProjectileClass)
  {
    UE_LOG(
      LogTemp,
      Warning,
      TEXT("ProjectilePool: ProjectileClass is NULL")
    );

    return;
  }

  // PoolSize만큼 Projectile을 미리 생성
  for (int32 i = 0; i < PoolSize; ++i)
  {
    AProjectileBase* Projectile =
      GetWorld()->SpawnActor<AProjectileBase>(
        ProjectileClass
      );

    // 정상적으로 생성되었다면
    if (Projectile)
    {
      // 처음에는 사용하지 않을 것이므로 비활성화
      Projectile->DeactivateProjectile();

      // Pool 배열에 저장
      ProjectilePool.Add(Projectile);
    }
  }

  UE_LOG(
    LogTemp,
    Warning,
    TEXT("ProjectilePool Created: %d"),
    ProjectilePool.Num()
  );
}

AProjectileBase* UProjectilePoolComponent::GetProjectile()
{
  // Pool에 있는 Projectile들을 하나씩 확인
  for (AProjectileBase* Projectile : ProjectilePool)
  {
    if (Projectile && Projectile->IsHidden())
    {
      // 사용 가능한 Projectile 반환
      return Projectile;
    }
  }

  return nullptr;
}