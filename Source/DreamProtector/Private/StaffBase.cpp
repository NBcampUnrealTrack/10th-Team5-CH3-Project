#include "StaffBase.h"
#include "ProjectileBase.h"
#include "PlayerCharacter.h"
#include "ProjectilePoolComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"


AStaffBase::AStaffBase()
{
  PrimaryActorTick.bCanEverTick = false;

  MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
  MuzzlePoint->SetupAttachment(StaticMeshComp);


}

void AStaffBase::ResetCastVFX()
{
  bCastVFXPlayed = false;
}

void AStaffBase::Attack()
{
  APlayerController* PC = GetWorld()->GetFirstPlayerController();
  if (!PC)
  {
    return;
  }

  // 현재 플레이어 캐릭터 가져오기
  APlayerCharacter* PlayerCharacter =
    Cast<APlayerCharacter>(PC->GetPawn());

  if (!PlayerCharacter)
  {
    return;
  }

  // 화면 크기 가져오기
  int32 ViewportX;
  int32 ViewportY;

  PC->GetViewportSize(
    ViewportX,
    ViewportY
  );

  // 화면 중앙을 월드 방향으로 변환
  FVector WorldLocation;
  FVector WorldDirection;

  PC->DeprojectScreenPositionToWorld(
    ViewportX * 0.5f,
    ViewportY * 0.5f,
    WorldLocation,
    WorldDirection
  );

  // 화면 중앙 기준 Trace
  FVector TraceStart = WorldLocation;
  FVector TraceEnd =
    TraceStart + WorldDirection * 10000.0f;

  FHitResult HitResult;

  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);
  Params.AddIgnoredActor(PlayerCharacter);

  bool bHit = GetWorld()->LineTraceSingleByChannel(
    HitResult,
    TraceStart,
    TraceEnd,
    ECC_Visibility,
    Params
  );

  // 조준 지점
  FVector AimPoint = bHit
    ? HitResult.ImpactPoint
    : TraceEnd;

  // 캐릭터 앞 CastPoint 위치
  FVector SpawnLocation =
    PlayerCharacter->GetCastPoint()->GetComponentLocation();

  // CastPoint → AimPoint 방향
  FVector FireDirection =
    (AimPoint - SpawnLocation).GetSafeNormal();

  FRotator SpawnRotation =
    FireDirection.Rotation();

  // 공격 한번에 마법진을 한 번만 생성
  if (!bCastVFXPlayed)
  {
    PlayCastVFX(
      SpawnLocation,
      SpawnRotation
    );

    bCastVFXPlayed = true;
  }

  // 플레이어가 가지고 있는 Projectile Pool 가져오기
  UProjectilePoolComponent* Pool =
    PlayerCharacter->GetProjectilePoolComponent();

  if (Pool)
  {
    // 현재 사용하지 않는 Projectile 하나 가져오기
    AProjectileBase* Projectile =
      Pool->GetProjectile();

    if (Projectile)
    {
      // 기존 Owner 충돌 무시 로직을 위해 Owner 지정
      Projectile->SetOwner(this);

      // 새로 생성하지 않고 기존 Projectile 재사용
      Projectile->ActivateProjectile(
        SpawnLocation,
        SpawnRotation
      );
    }
  }

  // 디버그용 조준
  DrawDebugLine(
    GetWorld(),
    SpawnLocation,
    AimPoint,
    FColor::Red,
    false,
    2.0f,
    0,
    2.0f
  );
}