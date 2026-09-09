#include "FlyingRanged.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

AFlyingRanged::AFlyingRanged()
{
	// 비행 모드로 설정 (중력 X, 상하 이동 가능)
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->GravityScale = 0.0f;
}

void AFlyingRanged::BeginPlay()
{
	// HP / Speed 초기화 
	Super::BeginPlay();

	// 스폰 위치 기준 지정된 높이만큼 위로 보정
	FVector StartLocation = GetActorLocation();
	StartLocation.Z += FlyHeight;
	SetActorLocation(StartLocation);
}

void AFlyingRanged::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 타겟(플레이어)가 없거나 죽으면 아무것도 하기 싫어요~
	if (!Target || CurrentHealth <= 0.0f) return;

	if (CheckObstacleAhead())
	{
		// 장애물 회피 우선
		AvoidObstacle(DeltaTime);
	}
	else
	{
		// 장애물 X -> 플레이어한테 ㄱㄱ
		MoveTowardsTarget(DeltaTime);
	}
	// 프레임마다 높이 조정
	MaintainFlightHeight(DeltaTime);

	if (IsTargetInAttackRange() && CanAttack())
	{
		Attack();
	}
}
void AFlyingRanged::MoveTowardsTarget(float DeltaTime)
{
	FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	// CharacterMovementComponent 로 이동 입력 적용
	AddMovementInput(Direction, 1.0f);
}

void AFlyingRanged::MaintainFlightHeight(float DeltaTime)
{
	// 현재 위치 가져오기
	FVector CurrentLocation = Target->GetActorLocation();
	// 지형 높이 반영하려면 라인트레이스로 바닥 감지 후 보정?
	const float DesiredZ = FlyHeight;

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

	// 투사체(AEnemyProjectile) 스폰 방식으로 갈지, 즉시 데미지 적용할지 결정 필요
	// 지금은 즉시 데미지 적용 방식으로 임시 구현
	// (Target 쪽에 TakeDamage(float) 함수가 있다는 전제)
}