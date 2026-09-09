#include "WeaponBase.h"


AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SceneRoot);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	Interact(this);
}

void AWeaponBase::Interact(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("Weapon Interact"));
}

void AWeaponBase::Attack()
{

}


