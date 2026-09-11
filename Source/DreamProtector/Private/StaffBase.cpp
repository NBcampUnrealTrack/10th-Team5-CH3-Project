#include "StaffBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"


AStaffBase::AStaffBase()
{
  PrimaryActorTick.bCanEverTick = false;

  MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
  MuzzlePoint->SetupAttachment(StaticMeshComp);

}

void AStaffBase::Attack()
{
  FVector SpawnLocation = MuzzlePoint->GetComponentLocation();

  APlayerController* PC = GetWorld()->GetFirstPlayerController();
  if (!PC)
  {
    return;
  }

  FVector CameraLocation;
  FRotator CameraRotation;

  PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

  //카메라방향
  FRotator SpawnRotation = CameraRotation;
  //방향 조정
  SpawnRotation.Pitch += 15.0f;
  SpawnRotation.Yaw -= 1.0f;

  if (ProjectileClass)
  {
    GetWorld()->SpawnActor<AProjectileBase>(
      ProjectileClass,
      SpawnLocation,
      SpawnRotation
    );
  }
}

