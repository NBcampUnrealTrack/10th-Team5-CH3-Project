#include "ProjectileBase.h"
#include "BaseMonster.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"


AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 발사체 충돌 감지 Collision 생성
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);

	SphereCollision->OnComponentHit.AddDynamic(
		this,
		&AProjectileBase::OnHit
	);

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

	//충돌한 Actor가 Monster라면 데미지 처리
	if (ABaseMonster* Monster = Cast<ABaseMonster>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster Hit: %s"), *Monster->GetName());

		//몬스터에게 데미지 전달
		Monster->TakeDamage(Damage);
		
		//피격 후 발사체를 오브젝트 풀로 반환
		DeactivateProjectile();

		return;
	}
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//비활성화 밫사체는 회전 안함
	if (!IsHidden())
	{
		//비행중 발사체 회전 적용
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
	//발사체를 발사 위치로 이동
	SetActorLocationAndRotation(
		SpawnLocation,
		SpawnRotation
	);

	//풀에 비활성화되어있던 발사체를 다시 보이게 하고 충돌 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	//Projectile Movement 활성화 및 발사 속도 적용
	if (ProjectileMovement)
	{
		ProjectileMovement->Activate(true);

		ProjectileMovement->Velocity =
			SpawnRotation.Vector() *
			ProjectileMovement->InitialSpeed;
	}

	// 아무것도 맞지 않은 발사체도
	// 3초 후 자동으로 오브젝트 풀에 반환
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
	// 자동 반환 타이머 제거
	GetWorldTimerManager().ClearTimer(DeactivateTimerHandle);

	// 발사체 숨기기 및 충돌 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 이동 중지 후 Projectile Movement 비활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}

void AProjectileBase::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}


	// 충돌 위치에 Niagara VFX 생성
	if (ImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactVFX,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation()
		);
	}
	// 실제 충돌 지점에서 Impact Sound 재생
	// ImpactAttenuation을 통해 거리에 따라 소리 크기가 감소
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			Hit.ImpactPoint,
			1.0f,
			1.0f,
			0.0f,
			ImpactAttenuation
		);
	}

	// 충돌 처리 후 발사체를 오브젝트 풀로 반환
	DeactivateProjectile();
}

void AProjectileBase::PlayImpactEffect(const FVector& Location, const FVector& Normal)
{
}
