#include "BossMonster.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
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

	OnBossHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	UpdatePhaseInBlackboard();

	// [테스트용] 트리거 볼륨 완성 전까지, 3초 뒤 자동으로 등장 씬 실행 - 나중에 이 두 줄 삭제
	GetWorldTimerManager().SetTimer(TestIntroTimerHandle, this, &ABossMonster::StartIntroSequence, 3.f, false);
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

	// AI를 멈추고 싶다면 여기서 AIController->GetBrainComponent()->StopLogic() 호출 고려

	if (PhaseTransitionMontage)
	{
		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(PhaseTransitionMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ABossMonster::OnPhaseTransitionMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, PhaseTransitionMontage);
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
	// AI 재개하려면 여기서 StartLogic() 호출
}

void ABossMonster::HandleDeath()
{
	if (bIsDead) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HandleDeath called!"));
	}

	bIsDead = true;
	bIsInvincible = true;

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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
				FString::Printf(TEXT("Spawning effect at: %s"), *GetActorLocation().ToString()));
		}
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathGroundEffect, GetActorLocation(), FRotator::ZeroRotator);
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

void ABossMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White,
			FString::Printf(TEXT("bIntroFinished=%s | State=%d | Time=%.2f"),
				bIntroFinished ? TEXT("TRUE") : TEXT("false"),
				(int32)CurrentIntroState,
				GetWorld()->GetTimeSeconds()));
	}

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

	if (CurrentIntroState == EBossIntroState::Pausing)
	{
		// 대기 중엔 위치 고정 (Flying 무브먼트 잔여 관성으로 밀리는 것 방지)
		SetActorLocation(LungeTargetLocation);
	}

	else if (CurrentIntroState == EBossIntroState::Rising)
	{
		// 성장과 상승이 모두 끝나면 등장 완료
		if (!bIsGrowing)
		{
			CurrentIntroState = EBossIntroState::Done;
			bIntroFinished = true;

			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool(TEXT("bIntroFinished"),true);
				}
			}
		}
	}

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
}

void ABossMonster::BeginRisingPhase()
{
	CurrentIntroState = EBossIntroState::Rising;

	if (RoarMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
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

	const FVector ToPlayer = PlayerPawn->GetActorLocation() - SpawnLocation;
	const FRotator SpawnRotation = ToPlayer.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(RangedProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
}