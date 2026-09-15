#include "StaffBase.h"
#include "ProjectileBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"


AStaffBase::AStaffBase()
{
  PrimaryActorTick.bCanEverTick = false;

  MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
  MuzzlePoint->SetupAttachment(StaticMeshComp);

}

void AStaffBase::Attack()
{
  APlayerController* PC = GetWorld()->GetFirstPlayerController();
  if (!PC)
  {
    return;
  }

  FVector CameraLocation;
  FRotator CameraRotation;

  PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

  // 카메라가 바라보는 방향으로 멀리 Trace
  FVector TraceStart = CameraLocation;
  FVector TraceEnd =
    TraceStart + CameraRotation.Vector() * 10000.0f;

  FHitResult HitResult;

  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  if (GetOwner())
  {
    Params.AddIgnoredActor(GetOwner());
  }

  bool bHit = GetWorld()->LineTraceSingleByChannel(
    HitResult,
    TraceStart,
    TraceEnd,
    ECC_Visibility,
    Params
  );

  // 맞은 게 있으면 그 위치
  // 없으면 카메라 정면 멀리 있는 위치
  FVector AimPoint = bHit
    ? HitResult.ImpactPoint
    : TraceEnd;

  // 실제 총알은 지팡이 끝에서 생성
  FVector SpawnLocation = MuzzlePoint->GetComponentLocation();

  // 지팡이 끝 -> 조준 지점 방향
  FVector FireDirection =
    (AimPoint - SpawnLocation).GetSafeNormal();

  FRotator SpawnRotation =
    FireDirection.Rotation();

  // 마법진 VFX
  //PlayCastVFX(SpawnLocation, SpawnRotation);

  if (ProjectileClass)
  {
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    GetWorld()->SpawnActor<AProjectileBase>(
      ProjectileClass,
      SpawnLocation,
      SpawnRotation,
      SpawnParams
    );
    DrawDebugLine(
      GetWorld(),
      TraceStart,
      AimPoint,
      FColor::Red,
      false,
      1.0f,
      0,
      1.0f
    );
    DrawDebugLine(
      GetWorld(),
      SpawnLocation,
      AimPoint,
      FColor::Green,
      false,
      1.0f,
      0,
      2.0f
    );
  }
}

