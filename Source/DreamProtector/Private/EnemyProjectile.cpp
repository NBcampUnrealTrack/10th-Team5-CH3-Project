#include "EnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	// CollisionComponent에 다른 Actor가 겹치면
	// OnProjectileOverlap 함수를 호출하도록 연결
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemyProjectile::OnProjectileOverlap
	);
}

AEnemyProjectile::AEnemyProjectile()
{
	// ProjectileMovement가 이동을 담당하므로
	// 이 Actor 자체는 Tick이 필요 없음
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌체 생성

	CollisionComponent =
		CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));

	// 이 충돌체를 투사체의 Root로 사용
	SetRootComponent(CollisionComponent);

	// 충돌 구체 크기
	CollisionComponent->InitSphereRadius(20.0f);

	// 충돌은 막는 방식이 아니라 겹침(Overlap) 방식으로 사용
	CollisionComponent->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	CollisionComponent->SetCollisionResponseToAllChannels(
		ECR_Overlap
	);
	// 2. 눈에 보이는 Mesh 생성
	ProjectileMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));

	ProjectileMesh->SetupAttachment(CollisionComponent);

	// 실제 충돌은 Sphere가 담당하므로
	// Mesh 자체 충돌은 끔
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// 3. 투사체 이동 컴포넌트
	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement")
		);

	// CollisionComponent를 움직이도록 지정
	ProjectileMovement->UpdatedComponent = CollisionComponent;

	// 처음 발사되는 속도
	ProjectileMovement->InitialSpeed = 800.0f;

	// 최대 속도
	ProjectileMovement->MaxSpeed = 800.0f;

	// 중력 영향 없음
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 4. 일정 시간 후 자동 제거
	InitialLifeSpan = 5.0f;
}

void AEnemyProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 자기 자신 또는 아무것도 아닌 경우 무시
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 부딪힌 Actor가 PlayerCharacter인지 확인
	APlayerCharacter* Player =
		Cast<APlayerCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	// 플레이어에게 데미지 전달
	Player->TakeDamageFromEnemy(Damage);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy Projectile Hit Player! Damage: %.1f"),
		Damage
	);

	// 플레이어를 맞혔으므로 투사체 제거
	Destroy();
}
