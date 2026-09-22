#include "FlyingRanged.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "EnemyProjectile.h"
#include "Sound/SoundBase.h"

AFlyingRanged::AFlyingRanged()
{
	// 비행 모드로 설정 (중력 X, 상하 이동 가능)
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->GravityScale = 0.0f;

	// 이동 방향이 아니라 직접 회전 제어할 거라 자동 회전 끔
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AFlyingRanged::BeginPlay()
{
	// HP / Speed 초기화 
	Super::BeginPlay();

	// 스폰 시점의 원래 높이(바닥 기준) 저장
	BaseGroundZ = GetActorLocation().Z;

	// 스폰 위치 기준 지정된 높이만큼 위로 보정
	FVector StartLocation = GetActorLocation();
	StartLocation.Z += FlyHeight;
	SetActorLocation(StartLocation);
}

void AFlyingRanged::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentHealth <= 0.0f)
	{
		return;
	}

	if (CheckObstacleAhead())
	{
		AvoidObstacle(DeltaTime);
	}

	if (Target)
	{
		// 이동 중이든 공격 중이든 계속 플레이어를 바라봄
		FaceTarget(DeltaTime);

		if (IsTargetInAttackRange())
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
	}
}

void AFlyingRanged::MoveTowardsTarget()
{
	if (!Target) return;
	if (IsTargetInAttackRange()) return;

	FVector TargetLocation = Target->GetActorLocation();

	// 플레이어보다 일정 높이 위를 목표로 함
	TargetLocation.Z += FlyHeight;

	FVector Direction =
		(TargetLocation - GetActorLocation()).GetSafeNormal();

	AddMovementInput(Direction, 1.0f);

	
}

void AFlyingRanged::MaintainFlightHeight(float DeltaTime)
{
	// 현재 위치 가져오기
	FVector CurrentLocation = GetActorLocation();
	// 지형 높이 반영하려면 라인트레이스로 바닥 감지 후 보정?
	const float DesiredZ = BaseGroundZ + FlyHeight;

	CurrentLocation.Z = FMath::FInterpTo(CurrentLocation.Z, DesiredZ, DeltaTime, 2.0f);
	SetActorLocation(CurrentLocation);
}

bool AFlyingRanged::CheckObstacleAhead()
{
	FVector Start = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector End = Start + Forward * ObstacleCheckDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	// 자기 자신 감지 대상 제외
	QueryParams.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

void AFlyingRanged::AvoidObstacle(float DeltaTime)
{
	// 간단 회피 - 오른쪽으로 살짝 움직임
	FVector AvoidDirction = GetActorRightVector();
	AddMovementInput(AvoidDirction, 1.0f);
}

bool AFlyingRanged::IsTargetInAttackRange() const
{
	return Target && GetDistanceTo(Target) <= AttackRange;
}

bool AFlyingRanged::CanAttack() const
{
	return GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown;
}

void AFlyingRanged::Attack_Implementation()
{
	
	// 공격 애니메이션
	if (!Target)
	{
		
		return;
	}
	// 쿨타임 갱신
	LastAttackTime = GetWorld()->GetTimeSeconds();

	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}

	// 정해진 소켓 위치에 파티클 재생 - 이펙트
	if (AttackEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(
			AttackEffect,
			GetMesh(),
			AttackEffectSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget
		);
	}

	// 공격 사운드 재생
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}

	// 발사할 투사체 클래스가 설정되지 않았다면 발사하지 않음
	if (!EnemyProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyProjectileClass is not set"));
		return;
	}

	// 총알이 생성될 위치
	FVector SpawnLocation =
		GetMesh()->GetSocketLocation(AttackEffectSocketName);

	// 플레이어 위치
	FVector TargetLocation =
		Target->GetActorLocation();

	// 플레이어 몸 중심 정도를 조준하도록 높이를 조금 올림
	TargetLocation.Z += 50.0f;

	// 발사 위치에서 플레이어를 향하는 방향 계산
	FVector FireDirection =
		(TargetLocation - SpawnLocation).GetSafeNormal();

	// 방향 벡터를 회전값으로 변환
	FRotator SpawnRotation =
		FireDirection.Rotation();

	// EnemyProjectile 생성
	AEnemyProjectile* Projectile =
		GetWorld()->SpawnActor<AEnemyProjectile>(
			EnemyProjectileClass,
			SpawnLocation,
			SpawnRotation
		);

	if (Projectile)
	{
		// 자기 자신과 투사체가 충돌하지 않도록
		Projectile->SetOwner(this);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("FlyingRanged Projectile Fired")
		);
	}
}

void AFlyingRanged::FaceTarget(float DeltaTime)
{
	if (!Target)
	{
		return;
	}

	FVector LookDirection =
		Target->GetActorLocation() - GetActorLocation();

	// 위아래로 기울지 않고 좌우로만 회전
	LookDirection.Z = 0.0f;

	if (LookDirection.IsNearlyZero())
	{
		return;
	}

	FRotator TargetRotation = LookDirection.Rotation();

	FRotator NewRotation = FMath::RInterpTo(
		GetActorRotation(),
		TargetRotation,
		DeltaTime,
		5.0f
	);

	SetActorRotation(NewRotation);
}