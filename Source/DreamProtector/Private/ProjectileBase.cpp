#include "ProjectileBase.h"
#include "PlayerCharacter.h"
#include "BaseMonster.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SphereCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	//처음 발사 속도
	ProjectileMovement->InitialSpeed = 5000.0f;
	//최대 속도
	ProjectileMovement->MaxSpeed = 5000.0f;
	//날아가는 방향을 바라보게 할거냐 true/false
	ProjectileMovement->bRotationFollowsVelocity = true;
	//중력 영향 (현재는 0 == 직선)
	ProjectileMovement->ProjectileGravityScale = 0.1f;

	//SphereCollision에서 Overlap이 시작 될때 현재 객체에서 OnOverlap 함수 실행
	SphereCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AProjectileBase::OnOverlapBegin
	);
}

void AProjectileBase::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//충돌한 Actor가 유효하지 않거나 나 자신이라면 무시
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (ABaseMonster* Monster = Cast<ABaseMonster>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster Hit: %s"), *Monster->GetName());

		float FinalDamage = Damage;

		// 이 투사체를 발사한 플레이어 찾기
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
		{
			FinalDamage *= Player->GetAttackDamageMultiplier();
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Projectile Damage: %.1f"),
			FinalDamage
		);

		Monster->TakeDamage(FinalDamage);
		
		DeactivateProjectile();

		return;
	}
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsHidden())
	{
		StaticMeshComp->AddLocalRotation(
			FRotator(350.0f, 150.0f, 90.0f) * DeltaTime
		);
	}
}

void AProjectileBase::ActivateProjectile(
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation
)
{
	SetActorLocationAndRotation(
		SpawnLocation,
		SpawnRotation
	);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (ProjectileMovement)
	{
		ProjectileMovement->Activate(true);

		ProjectileMovement->Velocity =
			SpawnRotation.Vector() *
			ProjectileMovement->InitialSpeed;
	}

	// 빗나간 총알도 3초 후 풀로 반환
	GetWorldTimerManager().SetTimer(
		DeactivateTimerHandle,
		this,
		&AProjectileBase::DeactivateProjectile,
		3.0f,
		false
	);
}

void AProjectileBase::DeactivateProjectile()
{
	GetWorldTimerManager().ClearTimer(DeactivateTimerHandle);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}
