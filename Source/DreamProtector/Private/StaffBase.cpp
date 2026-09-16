#include "StaffBase.h"
#include "ProjectileBase.h"
#include "PlayerCharacter.h"
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

  // 현재 플레이어 캐릭터 가져오기
  APlayerCharacter* PlayerCharacter =
    Cast<APlayerCharacter>(PC->GetPawn());

  if (!PlayerCharacter)
  {
    return;
  }

  // 카메라 위치 / 방향 가져오기
  FVector CameraLocation;
  FRotator CameraRotation;

  PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

  // 캐릭터 앞 CastPoint 위치에서 발사
  FVector SpawnLocation =
    PlayerCharacter->GetCastPoint()->GetComponentLocation();

  // 카메라가 바라보는 방향으로 Trace
  FVector TraceStart = CameraLocation;
  FVector TraceEnd =
    TraceStart + CameraRotation.Vector() * 10000.0f;

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

  // CastPoint → AimPoint 방향
  FVector FireDirection =
    (AimPoint - SpawnLocation).GetSafeNormal();

  FRotator SpawnRotation =
    FireDirection.Rotation();

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

