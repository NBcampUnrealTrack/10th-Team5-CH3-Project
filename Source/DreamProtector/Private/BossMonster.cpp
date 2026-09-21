#include "BossMonster.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	if (bIsDead)
	{
		return;
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

	if (CurrentIntroState == EBossIntroState::Lunging)
	{
		//const FVector NewLoc = FMath::VInterpConstantTo(GetActorLocation(), LungeTargetLocation, DeltaTime, IntroMoveSpeed);
		//SetActorLocation(NewLoc);

		//if (FVector::Dist(GetActorLocation(), LungeTargetLocation) <= IntroArrivalTolerance)
		//{
		//	// 돌진 도착 -> 바로 상승하지 않고 잠깐 대기 (Pausing 상태로 전환)
		//	CurrentIntroState = EBossIntroState::Pausing;
		//	SetActorLocation(LungeTargetLocation);

		//	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		//	{
		//		MoveComp->StopMovementImmediately();
		//	}

		//	GetWorldTimerManager().SetTimer(IntroPauseTimerHandle, this, &ABossMonster::BeginRisingPhase, IntroPauseDuration, false);
		//}
	}

	//else if (CurrentIntroState == EBossIntroState::Pausing)
	if (CurrentIntroState == EBossIntroState::Pausing)
	{
		// 대기 중엔 위치 고정 (Flying 무브먼트 잔여 관성으로 밀리는 것 방지)
		SetActorLocation(LungeTargetLocation);
	}

	else if (CurrentIntroState == EBossIntroState::Rising)
	{
		const FVector NewLoc = FMath::VInterpConstantTo(GetActorLocation(), AnchorLocation, DeltaTime, IntroMoveSpeed);
		SetActorLocation(NewLoc);

		if (FVector::Dist(GetActorLocation(), AnchorLocation) <= IntroArrivalTolerance)
		{
			// 상승 완료 -> 등장 씬 종료
			CurrentIntroState = EBossIntroState::Done;
			bIntroFinished = true;

			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool(TEXT("bIntroFinished"), true);
				}
			}
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
}

//void ABossMonster::StartIntroSequence()
//{
//	// 이미 시작했거나 끝났으면 중복 실행 방지
//	if (CurrentIntroState != EBossIntroState::NotStarted)
//	{
//		return;
//	}
//
//	CurrentIntroState = EBossIntroState::Lunging;
//
//	if (LungeMontage)
//	{
//		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
//		{
//			AnimInstance->Montage_Play(LungeMontage);
//		}
//	}
//}

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
