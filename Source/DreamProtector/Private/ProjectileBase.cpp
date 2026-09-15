#include "ProjectileBase.h"
#include "BaseMonster.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SphereCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	//처음 발사 속도
	ProjectileMovement->InitialSpeed = 3000.0f;
	//최대 속도
	ProjectileMovement->MaxSpeed = 3000.0f;
	//날아가는 방향을 바라보게 할거냐 true/false
	ProjectileMovement->bRotationFollowsVelocity = true;
	//중력 영향 (현재는 0 == 직선)
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	//발사체 수명 (1초뒤 삭제) 
	InitialLifeSpan = 1.0f;

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

		Monster->TakeDamage(Damage);
		
		Destroy();

		return;
	}
}




