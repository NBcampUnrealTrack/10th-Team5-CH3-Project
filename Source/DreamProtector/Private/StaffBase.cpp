#include "StaffBase.h"


AStaffBase::AStaffBase()
{
  PrimaryActorTick.bCanEverTick = false;

  MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
  MuzzlePoint->SetupAttachment(StaticMeshComp);

}

void AStaffBase::Attack()
{
  //스태프에 Muzzle 현재 위치 가져오기
  FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
  //스태프에 Muzzle 현재 방향 가져오기
  FRotator SpawnRotation = MuzzlePoint->GetComponentRotation();
  //투사체 클래스가 지정되어있으면 실행
  if (ProjectileClass)
  { 
    //현재 게임월드에 AProjectileBase 액터 생성
    GetWorld()->SpawnActor<AProjectileBase>(
      // 무었을 생성할지
      ProjectileClass,
      // 어디에 생성할지
      SpawnLocation,
      // 어떤 방향으로 생성할지
      SpawnRotation
    );
  }
}

void AStaffBase::BeginPlay()
{
  Super::BeginPlay();

  Attack();
}
