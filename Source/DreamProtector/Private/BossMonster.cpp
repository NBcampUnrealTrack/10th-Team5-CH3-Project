#include "BossMonster.h"
#include "AIController.h"
#include "PlayerCharacter.h"
#include "BrainComponent.h"
#include "BossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"


ABossMonster::ABossMonster()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABossMonster::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	CurrentPhase = EBossPhase::Phase1;

	// 보스는 항상 공중에 떠있는 상태 - Flying 모드 + 중력 무시
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->GravityScale = 0.0f;
	}
	// 레벨에 배치된 태그 액터 위치를 앵커 / 돌진 목표로 세팅 (못 찾으면 현재 위치로 폴백)
	AnchorLocation = GetActorLocation();
	LungeTargetLocation = GetActorLocation();

	TArray<AActor*> AnchorActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("BossAnchor")), AnchorActors);
	if (AnchorActors.Num() > 0)
	{
		AnchorLocation = AnchorActors[0]->GetActorLocation();
	}

	TArray<AActor*> LungeActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("BossLungeTarget")), LungeActors);
	if (LungeActors.Num() > 0)
	{
		LungeTargetLocation = LungeActors[0]->GetActorLocation();
	}

	TArray<AActor*> DeathGroundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("BossDeathGround")), DeathGroundActors);
	if (DeathGroundActors.Num() > 0)
	{
		DeathGroundLocation = DeathGroundActors[0]->GetActorLocation();
	}
	else
	{
		DeathGroundLocation = GetActorLocation();
	}

	TArray<AActor*> LaserGroundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("BossLaserGround")), LaserGroundActors);
	if (LaserGroundActors.Num() > 0)
	{
		LaserGroundLocation = LaserGroundActors[0]->GetActorLocation();
	}
	else
	{
		// 못 찾으면 사망 이펙트 바닥 위치 + 100으로 폴백
		LaserGroundLocation = DeathGroundLocation + FVector(0.f, 0.f, 100.f);
	}

	OnBossHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	UpdatePhaseInBlackboard();

	// [테스트용] 트리거 볼륨 완성 전까지, 3초 뒤 자동으로 등장 씬 실행 - 나중에 이 두 줄 삭제
	// GetWorldTimerManager().SetTimer(TestIntroTimerHandle, this, &ABossMonster::StartIntroSequence, 3.f, false);
}

FVector ABossMonster::GetAnchorLocation() const
{
	return AnchorLocation;
}

float ABossMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsInvincible || bIsDead)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	OnBossHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// 넉백, 스턴 없고 피격 사운드 + 이펙트
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	if (HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, GetActorLocation());
	}

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath();
	}
	else
	{
		CheckPhaseTransition();
	}

	return ActualDamage;
}

void ABossMonster::CheckPhaseTransition()
{
	const float HealthPct = CurrentHealth / MaxHealth;

	if (CurrentPhase == EBossPhase::Phase1 && HealthPct <= Phase2Threshold)
	{
		StartPhaseTransition(EBossPhase::Phase2);
	}
	else if (CurrentPhase == EBossPhase::Phase2 && HealthPct <= Phase3Threshold)
	{
		StartPhaseTransition(EBossPhase::Phase3);
	}
}

void ABossMonster::StartPhaseTransition(EBossPhase NewPhase)
{
	bIsInvincible = true;
	UpdateInvincibleInBlackboard(true);
	PendingPhase = NewPhase;

	// 페이즈별 전환 몽타주 선택 (슬롯이 비어 있으면 연출 없이 바로 전환)
	UAnimMontage* TransitionMontage = nullptr;
	if (NewPhase == EBossPhase::Phase2)
	{
		TransitionMontage = Phase2TransitionMontage;
	}
	else if (NewPhase == EBossPhase::Phase3)
	{
		TransitionMontage = Phase3TransitionMontage;
	}

	if (TransitionMontage)
	{
		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(TransitionMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ABossMonster::OnPhaseTransitionMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, TransitionMontage);
			return;
		}
	}

	// 모션이 없다면 즉시 전화 (필요 시 파티클이라도 여기서 재생)
	OnPhaseTransitionMontageEnded(nullptr, false);
	CurrentPhase = NewPhase;
	OnBossPhaseChanged.Broadcast(CurrentPhase);
	UpdatePhaseInBlackboard();
	bIsInvincible = false;
}

void ABossMonster::OnPhaseTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsInvincible = false;
	UpdateInvincibleInBlackboard(false);
	// AI 재개하려면 여기서 StartLogic() 호출

	CurrentPhase = PendingPhase;
	OnBossPhaseChanged.Broadcast(CurrentPhase);
	UpdatePhaseInBlackboard();

	// 3페이즈 진입 시 회전 레이저 시작
	if (CurrentPhase == EBossPhase::Phase3)
	{
		StartLaserPhase();
	}
}

void ABossMonster::HandleDeath()
{
	if (bIsDead) return;

	bIsDead = true;
	bIsInvincible = true;

	// 2페이즈 장판 정리 (예고 마법진 제거 + 폭발 타이머 취소)
	GetWorldTimerManager().ClearTimer(GroundBlastTimerHandle);
	for (UNiagaraComponent* WarningFX : PendingBlastWarnings)
	{
		if (WarningFX)
		{
			WarningFX->DestroyComponent();
		}
	}
	PendingBlastWarnings.Reset();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (AIController->GetBrainComponent())
		{
			AIController->GetBrainComponent()->StopLogic(TEXT("BossDied"));
		}
	}

	// [테스트용] 일단 트레이스 없이 보스 위치에 바로 재생
	if (DeathGroundEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathGroundEffect, DeathGroundLocation, FRotator::ZeroRotator);
	}

	/* 발밑에 사망 이펙트(마법진 등) 재생 - 아래로 트레이스해서 실제 바닥 위치를 찾음
	if (DeathGroundEffect)
	{
		FVector TraceStart = GetActorLocation();
		FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 5000.f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FVector EffectLocation = TraceStart;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			EffectLocation = HitResult.Locawtion;
		}

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathGroundEffect, EffectLocation, FRotator::ZeroRotator);
	}*/

	if (DeathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DeathMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ABossMonster::OnDeathMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
			return;
		}
	}

	// 몽타주 없으면 바로 제거
	OnDeathMontageEnded(nullptr, false);
}

void ABossMonster::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnBossDied.Broadcast();
	Destroy();
}

void ABossMonster::UpdatePhaseInBlackboard()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsInt(TEXT("CurrentPhase"), static_cast<int32>(CurrentPhase));
		}
	}
}

void ABossMonster::UpdateInvincibleInBlackboard(bool bValue)
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsBool(TEXT("bIsInvincible"), bValue);
		}
	}
}

void ABossMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGrowing && GetMesh())
	{
		GrowElapsedTime += DeltaTime;

		const float Alpha = GrowDuration > 0.0f
			? FMath::Clamp(
				GrowElapsedTime / GrowDuration,
				0.0f,
				1.0f
			)
			: 1.0f;

		const float SmoothAlpha =
			Alpha * Alpha * (3.0f - 2.0f * Alpha);

		// 크기와 위치에 동일한 진행률 사용
		GetMesh()->SetRelativeScale3D(
			FMath::Lerp(
				GrowStartScale,
				FVector(TargetMeshScale),
				SmoothAlpha
			)
		);

		SetActorLocation(
			FMath::Lerp(
				GrowStartLocation,
				AnchorLocation,
				SmoothAlpha
			)
		);

		if (Alpha >= 1.0f)
		{
			GetMesh()->SetRelativeScale3D(
				FVector(TargetMeshScale)
			);
			SetActorLocation(AnchorLocation);

			bIsGrowing = false;
		}

		if (Alpha >= 1.0f)
		{
			GetMesh()->SetRelativeScale3D(
				FVector(TargetMeshScale)
			);
			SetActorLocation(AnchorLocation);

			bIsGrowing = false;

			// 콜리전(캡슐)도 메시 크기에 맞춰 확대 - 안 그러면 발사체가 비주얼상 몸통을 그냥 통과함
			if (UCapsuleComponent* Capsule = GetCapsuleComponent())
			{
				Capsule->SetCapsuleSize(
					Capsule->GetUnscaledCapsuleRadius() * TargetMeshScale,
					Capsule->GetUnscaledCapsuleHalfHeight() * TargetMeshScale
				);
			}
		}
	}
	// 트리거 넣으려고 통으로 수정했습니다.
	if (CurrentIntroState == EBossIntroState::Pausing)
	{
		// 대기 중엔 위치 고정 (Flying 무브먼트 잔여 관성으로 밀리는 것 방지)
		SetActorLocation(LungeTargetLocation);
	}

	if (CurrentIntroState == EBossIntroState::Rising)
	{
		if (!bIsGrowing)
		{
			CurrentIntroState = EBossIntroState::Done;
			bIntroFinished = true;

			bIsInvincible = false;

			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				// BT를 먼저 시작시키고
				if (ABossAIController* BossAIController = Cast<ABossAIController>(AIController))
				{
					BossAIController->StartBossBattle();
				}

				// 그다음에 블랙보드 값을 세팅 (BT가 시작된 이후에 써야 리셋 안 됨)
				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool(TEXT("bIntroFinished"), true);
				}
			}
		}
	}
	//if (CurrentIntroState == EBossIntroState::Rising)
	//{
		// 성장과 상승이 모두 끝나면 등장 완료
	//	if (!bIsGrowing)
		//{
			//CurrentIntroState = EBossIntroState::Done;
			//bIntroFinished = true;

//			if (AAIController* AIController =
	//			Cast<AAIController>(GetController()))
		//	{
			//	if (UBlackboardComponent* BB =
				//	AIController->GetBlackboardComponent())
				//{
				//	BB->SetValueAsBool(TEXT("bIntroFinished"), true);
				//}
			//}
		//}
	//}
	// 등장 씬 종료 후엔 매 프레임 플레이어 쪽으로 회전 (Pausing/Rising과 별개로 항상 체크)
	if (CurrentIntroState == EBossIntroState::Done)
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
			const FRotator TargetRot = FRotator(0.f, ToPlayer.Rotation().Yaw, 0.f);
			const FRotator NewRot = FMath::RInterpConstantTo(GetActorRotation(), TargetRot, DeltaTime, FaceTargetRotationSpeed);
			SetActorRotation(NewRot);
		}
	}

	if (bLaserActive && LaserPivot)
	{
		LaserPivot->AddLocalRotation(FRotator(0.f, LaserRotationSpeed * DeltaTime, 0.f));
	}

	if (bLaserActive && LaserPivot)
	{
		LaserPivot->SetWorldLocation(FVector(
			GetActorLocation().X,
			GetActorLocation().Y,
			LaserGroundLocation.Z));

		LaserPivot->AddLocalRotation(FRotator(0.f, LaserRotationSpeed * DeltaTime, 0.f));
	}
}

void ABossMonster::BeginRisingPhase()
{
	CurrentIntroState = EBossIntroState::Rising;

	if (GetMesh())
	{
		GrowStartScale = GetMesh()->GetRelativeScale3D();
		GrowStartLocation = GetActorLocation();

		GrowElapsedTime = 0.0f;
		bIsGrowing = true;
	}

	if (RoarMontage)
	{
		if (UAnimInstance* AnimInstance =
			GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(RoarMontage);
		}
	}

	// GrowDelay만큼 기다린 후 성장 시작
	FTimerHandle GrowStartTimerHandle;
	GetWorldTimerManager().SetTimer(GrowStartTimerHandle, this, &ABossMonster::BeginGrowing, GrowDelay, false);
}

void ABossMonster::BeginGrowing()
{
	bIsGrowing = true;
	GrowElapsedTime = 0.f;
	GrowStartScale = GetMesh() ? GetMesh()->GetRelativeScale3D() : FVector::OneVector;
	GrowStartLocation = GetActorLocation();
}

void ABossMonster::StartIntroSequence()
{
	if (CurrentIntroState != EBossIntroState::NotStarted)
	{
		return;
	}

	CurrentIntroState = EBossIntroState::Lunging;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	UAnimInstance* AnimInstance =
		GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (AnimInstance && LungeMontage)
	{
		if (AnimInstance->Montage_Play(LungeMontage) > 0.0f)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(
				this, &ABossMonster::OnLungeMontageEnded);

			AnimInstance->Montage_SetEndDelegate(
				EndDelegate, LungeMontage);

			return;
		}
	}

	// 애니메이션을 재생할 수 없다면 현재 위치에서 다음 단계로 진행
	OnLungeMontageEnded(nullptr, false);
}

void ABossMonster::OnLungeMontageEnded(
	UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsDead ||
		CurrentIntroState != EBossIntroState::Lunging ||
		bInterrupted)
	{
		return;
	}

	// 애니메이션으로 실제 도착한 위치를 대기 위치로 사용
	LungeTargetLocation = GetActorLocation();
	CurrentIntroState = EBossIntroState::Pausing;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	if (IntroPauseDuration <= 0.0f)
	{
		BeginRisingPhase();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			IntroPauseTimerHandle,
			this,
			&ABossMonster::BeginRisingPhase,
			IntroPauseDuration,
			false);
	}
}

void ABossMonster::FireRangedAttack()
{
	// 사운드는 애니메이션보다 1초 늦게 재생
	GetWorldTimerManager().SetTimer(RangedAttackSoundTimerHandle, this, &ABossMonster::PlayRangedAttackSound, 1.f, false);

	if (RangedAttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(RangedAttackMontage);
		}
	}

	// 시전 동작 재생 후 딜레이 뒤에 실제 발사
	GetWorldTimerManager().SetTimer(RangedAttackTimerHandle, this, &ABossMonster::SpawnRangedProjectile, RangedAttackCastDelay, false);
}

void ABossMonster::PlayRangedAttackSound()
{
	if (RangedAttackCastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RangedAttackCastSound, GetActorLocation());
	}
}

void ABossMonster::SpawnRangedProjectile()
{
	if (!RangedProjectileClass)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// 낫 소켓 위치에서, 낫/보스 메시 콜리전을 피하기 위해 앞으로 + 위로 더 밀어서 발사
	FVector SocketLocation = GetActorLocation();
	if (GetMesh() && GetMesh()->DoesSocketExist(TEXT("hand_rSocket_Scythe")))
	{
		SocketLocation = GetMesh()->GetSocketLocation(TEXT("hand_rSocket_Scythe"));
	}

	const FVector SpawnLocation = SocketLocation
		+ GetActorForwardVector() * RangedSpawnForwardOffset
		+ FVector(0.f, 0.f, RangedSpawnUpOffset);

	// 0.3초 정도 플레이어를 앞서는 위치에 공격
	const FVector PredictedTarget = PlayerPawn->GetActorLocation()
		+ PlayerPawn->GetVelocity() * 0.3f;
	const FVector ToPlayer = PlayerPawn->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = ToPlayer.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(RangedProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
}

bool ABossMonster::TryStartMeleeAttack()
{
	// 이미 공격 애니메이션 재생 중이면, 그냥 계속 성공으로 처리해서 재시작 안 시킴
	if (bIsMeleeAttacking)
	{
		return true;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return false;
	}

	const float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > MeleeAttackRange)
	{
		return false;
	}

	bIsMeleeAttacking = true;

	// 혹시 남아있는 원거리 공격 예약(발사체 스폰 등)이 있다면 취소
	GetWorldTimerManager().ClearTimer(RangedAttackTimerHandle);
	GetWorldTimerManager().ClearTimer(RangedAttackSoundTimerHandle);

	if (MeleeAttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(MeleeAttackMontage);
		}
	}

	
	// 낫 휘두르는 사운드는 애니메이션보다 0.8초 늦게 재생
	GetWorldTimerManager().SetTimer(MeleeSwingSoundTimerHandle, this, &ABossMonster::PlayMeleeSwingSound, 0.8f, false);

	GetWorldTimerManager().SetTimer(MeleeAttackTimerHandle, this, &ABossMonster::ApplyMeleeHit, MeleeAttackCastDelay, false);

	return true;
}

void ABossMonster::ApplyMeleeHit()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// 타격 판정 시점에 다시 한번 거리 체크 (그 사이 플레이어가 도망갔을 수도 있으니)
	const float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > MeleeAttackRange * 1.2f)
	{
		return;
	}

	// UGameplayStatics::ApplyDamage(PlayerPawn, MeleeDamage, nullptr, this, nullptr);
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerPawn))
	{
		Player->TakeDamageFromEnemy(MeleeDamage);
	}

	// 타격 성공 시 사운드 재생
	if (MeleeHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MeleeHitSound, PlayerPawn->GetActorLocation());
	}

	// 넉백: 보스 -> 플레이어 방향으로 살짝 띄우면서 밀어냄
	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
	{
		FVector KnockbackDir = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		KnockbackDir.Z = 0.4f; // 살짝 위로도 띄우기
		KnockbackDir.Normalize();

		PlayerCharacter->LaunchCharacter(KnockbackDir * MeleeKnockbackForce, true, true);
	}

	// 몽타주 재생이 끝날 시점에 공격 상태 해제 (남은 재생 시간만큼 대기)
	const float RemainingTime = MeleeAttackMontage ? FMath::Max(MeleeAttackMontage->GetPlayLength() - MeleeAttackCastDelay, 0.2f) : 0.5f;
	GetWorldTimerManager().SetTimer(MeleeAttackEndTimerHandle, this, &ABossMonster::EndMeleeAttack, RemainingTime, false);
}

void ABossMonster::EndMeleeAttack()
{
	bIsMeleeAttacking = false;
}

void ABossMonster::PlayMeleeSwingSound()
{
	if (MeleeSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MeleeSwingSound, GetActorLocation());
	}
}

bool ABossMonster::StartGroundBlast()
{
	if (bIsGroundBlasting || bIsDead)
	{
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return false;
	}

	bIsGroundBlasting = true;
	PendingBlastLocations.Reset();
	PendingBlastWarnings.Reset();

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

	// 첫 장판은 플레이어 발밑, 나머지는 주변 랜덤
	for (int32 i = 0; i < GroundBlastCount; ++i)
	{
		FVector Loc = PlayerLoc;
		if (i > 0)
		{
			const float Angle = FMath::FRandRange(0.f, 360.f);
			const float Dist = FMath::FRandRange(GroundBlastRadius, GroundBlastSpreadRadius);
			Loc += FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
				FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist, 0.f);
		}

		// 바닥 높이로 맞추기 (레이저 높이 타겟 포인트의 Z를 재활용)
		Loc.Z = DeathGroundLocation.Z;

		PendingBlastLocations.Add(Loc);

		if (GroundBlastWarningFX)
		{
			UNiagaraComponent* Warning = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), GroundBlastWarningFX, Loc, FRotator::ZeroRotator, GroundBlastWarningScale);
			PendingBlastWarnings.Add(Warning);
		}
	}

	GetWorldTimerManager().SetTimer(GroundBlastTimerHandle, this, &ABossMonster::ExplodeGroundBlast, GroundBlastDelay, false);
	return true;
}

void ABossMonster::ExplodeGroundBlast()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	for (int32 i = 0; i < PendingBlastLocations.Num(); ++i)
	{
		const FVector& Loc = PendingBlastLocations[i];

		// 경고 마법진 제거
		if (PendingBlastWarnings.IsValidIndex(i) && PendingBlastWarnings[i])
		{
			PendingBlastWarnings[i]->DestroyComponent();
		}

		// 폭발 이펙트 + 사운드
		if (GroundBlastExplosionFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GroundBlastExplosionFX,
				Loc, FRotator::ZeroRotator, GroundBlastExplosionScale);
		}
		if (GroundBlastExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, GroundBlastExplosionSound, Loc);
		}

		// 판정: 수평 거리만 비교 (플레이어 높이 차이는 무시)
		if (PlayerPawn)
		{
			const FVector P = PlayerPawn->GetActorLocation();
			if (FVector::Dist2D(P, Loc) <= GroundBlastRadius)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerPawn))
				{
					Player->TakeDamageFromEnemy(GroundBlastDamage);
				}
			}
		}
	}

	PendingBlastLocations.Reset();
	PendingBlastWarnings.Reset();
	bIsGroundBlasting = false;
}

void ABossMonster::StartLaserPhase()
{
	if (bLaserActive)
	{
		return;
	}
	bLaserActive = true;

	// 회전 중심(Pivot)을 보스에 붙임 - 보스 위치를 그대로 따라감
	LaserPivot = NewObject<USceneComponent>(this, TEXT("LaserPivot"));
	LaserPivot->SetupAttachment(RootComponent);
	LaserPivot->RegisterComponent();
	
	// 보스의 위치/회전/스케일을 물려받지 않게 하고, 레이저 높이에 고정
	LaserPivot->SetAbsolute(true, true, true);
	LaserPivot->SetWorldLocation(FVector(
		GetActorLocation().X,
		GetActorLocation().Y,
		LaserGroundLocation.Z));
	LaserPivot->SetWorldRotation(FRotator::ZeroRotator);

	// 4방향(0/90/180/270도)으로 빔 4개 생성
	for (int32 i = 0; i < 4; ++i)
	{
		const float Yaw = 90.f * i;

		UBoxComponent* Beam = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("LaserBeam_%d"), i));
		Beam->SetupAttachment(LaserPivot);
		Beam->RegisterComponent();
		Beam->SetBoxExtent(FVector(LaserLength * 0.5f, LaserThickness * 0.5f * LaserHitboxScale, LaserHeight * 0.5f * LaserHitboxScale));
		Beam->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
		Beam->SetRelativeLocation(FVector(LaserLength * 0.5f, 0.f, 0.f).RotateAngleAxis(Yaw, FVector::UpVector));

		Beam->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Beam->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Beam->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		Beam->SetGenerateOverlapEvents(true);

		// 비주얼 메시 (있으면)
		if (LaserBeamMesh)
		{
			UStaticMeshComponent* VisualMesh = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("LaserVisual_%d"), i));
			VisualMesh->SetupAttachment(Beam);
			VisualMesh->RegisterComponent();
			VisualMesh->SetStaticMesh(LaserBeamMesh);
			VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			const FVector MeshSize = LaserBeamMesh->GetBounds().BoxExtent * 2.f;
			VisualMesh->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
			VisualMesh->SetRelativeScale3D(FVector(
				LaserThickness / FMath::Max(MeshSize.X, 1.f),   // 굵기
				LaserLength / FMath::Max(MeshSize.Y, 1.f),   // 길이
				LaserHeight / FMath::Max(MeshSize.Z, 1.f))); // 높이
		}

		LaserBeams.Add(Beam);
	}

	// 일정 간격마다 데미지 판정
	GetWorldTimerManager().SetTimer(LaserDamageTimerHandle, this, &ABossMonster::ApplyLaserDamageTick, LaserDamageInterval, true);
}

void ABossMonster::ApplyLaserDamageTick()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	for (UBoxComponent* Beam : LaserBeams)
	{
		if (!Beam)
		{
			continue;
		}

		TArray<AActor*> OverlappingActors;
		Beam->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

		if (OverlappingActors.Contains(PlayerPawn))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerPawn))
			{
				Player->TakeDamageFromEnemy(LaserDamagePerTick);
			}
			break;
		}
	}
}

void ABossMonster::StopLaserPhase()
{
	bLaserActive = false;
	GetWorldTimerManager().ClearTimer(LaserDamageTimerHandle);
}
