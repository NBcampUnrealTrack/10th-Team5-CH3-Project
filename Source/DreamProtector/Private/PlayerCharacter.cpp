#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Interactable.h"
#include "WeaponBase.h"
#include "StaffBase.h"
#include "InventoryComponent.h"
#include "ProjectilePoolComponent.h"
#include "Components/ChildActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFrameWork/SpringArmComponent.h"

APlayerCharacter::APlayerCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	//스프링암 컴포넌트 추가
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.0f, 50.0f, 50.0f);

	// 벽 충돌 대응
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeChannel = ECC_Camera;
	SpringArm->ProbeSize = 12.0f;

	//카메라 렉 사용하여 뒤늦게 따라오기 (이동)
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.0f;
	//회전
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 12.0f;

	//카메라 컴포넌트추가
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	Camera->SetRelativeLocation(FVector::ZeroVector);
	Camera->SetRelativeRotation(FRotator::ZeroRotator);

	//인벤토리 컴포넌트 추가
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CastPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CastPoint"));
	CastPoint->SetupAttachment(RootComponent);

	ProjectilePoolComponent = CreateDefaultSubobject<UProjectilePoolComponent>(TEXT("ProjectilePoolComponent"));

	// 캐릭터 기준 앞쪽으로 이동
	CastPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 50.0f));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentMana = MaxMana;

	// BP_PlayerCharacter에 붙어있는 Child Actor Component 가져오기
	UChildActorComponent* StaffWeaponComponent =
		FindComponentByClass<UChildActorComponent>();

	if (StaffWeaponComponent)
	{
		// Child Actor Component 안의 실제 BP_StaffBase Actor 가져오기
		AActor* StaffActor =
			StaffWeaponComponent->GetChildActor();

		CurrentWeapon =
			Cast<AWeaponBase>(StaffActor);

		if (CurrentWeapon)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Weapon Found: %s"),
				*CurrentWeapon->GetName()
			);
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//실제 플레이어 이동함수
void APlayerCharacter::Move(const FVector2D& MovementVector)
{
	AddMovementInput(GetActorForwardVector(), MovementVector.X);
	AddMovementInput(GetActorRightVector(), MovementVector.Y);
}

//실제 플레이어 이동속도 스프린트값으로 증가
void APlayerCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
//실제 플레이어 이동속도 다시 평소대로 감소
void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

}
//ABP
void APlayerCharacter::FireCurrentWeapon()
{
	if (!CurrentWeapon)
	{
		return;
	}

	CurrentWeapon->Attack();
}
//ABP
void APlayerCharacter::EndAttackAnimation()
{
	bIsAttacking = false;
	UE_LOG(LogTemp, Warning, TEXT("EndAttackAnimation Called"));
}

void APlayerCharacter::StartAttack()
{
	// 쿨타임 중이면 공격 불가
	if (!bCanAttack)
	{
		return;
	}

	// 이전 공격 모션이 아직 끝나지 않았다면 공격 불가
	if (bIsAttacking)
	{
		return;
	}

	// 장전 중이면 공격 불가
	if (bIsReloading)
	{
		return;
	}

	// 무기가 없다면 공격 불가
	if (!CurrentWeapon)
	{
		return;
	}

	// 마나 소모
	if (!ConsumeMana())
	{
		return;
	}

	// 공격 상태 시작
	bIsAttacking = true;
	bIsAutoFiring = false;

	// 공격 몽타주 재생
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}

	// 단발 쿨타임
	bCanAttack = false;

	GetWorldTimerManager().SetTimer(
		SingleFireCooldownTimer,
		[this]()
		{
			bCanAttack = true;
		},
		SingleFireCooldown,
		false
	);

	// 일정 시간 이상 누르면 연사 시작
	GetWorldTimerManager().SetTimer(
		AutoFireStartTimerHandle,
		this,
		&APlayerCharacter::StartAutoFire,
		AutoFireHoldTime,
		false
	);
}

void APlayerCharacter::StartAutoFire()
{
	bIsAutoFiring = true;
	bIsAttacking = true;

	// 연사 상체 모션 시작
	if (AutoFireMontage)
	{
		PlayAnimMontage(AutoFireMontage);
	}

	GetWorldTimerManager().SetTimer(
		AutoFireTimerHandle,
		this,
		&APlayerCharacter::AutoAttack,
		AttackInterval,
		true
	);
}

void APlayerCharacter::AutoAttack()
{
	if (!CurrentWeapon)
	{
		StopAttack();
		return;
	}

	// 마나 없으면 연사 종료
	if (!ConsumeMana())
	{
		StopAttack();
		return;
	}

	CurrentWeapon->Attack();
}

void APlayerCharacter::StopAttack()
{
	// StopAttack 호출 시점에 연사 중이었는지 기억
	bool bWasAutoFiring = bIsAutoFiring;

	// 연사 시작 대기 취소
	GetWorldTimerManager().ClearTimer(AutoFireStartTimerHandle);

	// 연사 종료
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	bIsAutoFiring = false;

	// 실제 연사 상태였다면
	// Attack01의 EndAttack Notify를 못 거쳤을 수 있으므로 직접 종료
	if (bWasAutoFiring)
	{
		bIsAttacking = false;

		if (AutoFireMontage)
		{
			StopAnimMontage(AutoFireMontage);
		}
	}

	if (AStaffBase* Staff = Cast<AStaffBase>(CurrentWeapon))
	{
		Staff->ResetCastVFX();
	}
}

	void APlayerCharacter::TryInteract()
	{
		//플레이어 위치에서 정면 300거리까지 상호작용 탐색
		FVector Start = GetActorLocation();
		FVector End = Start + GetActorForwardVector() * 300.0f;

		//Line Trace 결과 저장
		FHitResult HitResult;

		//조건 설정
		FCollisionQueryParams Params;

		//자기 자신에게 충돌하지 않도록
		Params.AddIgnoredActor(this);

		// 반지름 100의 구를 Start ~ End까지 이동시켜 충돌한 Actor 탐색
		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			Start,
			End,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(100.0f),
			Params
		);

		// 테스트용 상호작용 범위 확인
		DrawDebugSphere(
			GetWorld(),
			Start,
			100.0f,
			16,
			FColor::Red,
			false,
			2.0f
		);


		// 감지했다면
		if (bHit)
		{
			//Actor 가져오기
			AActor* HitActor = HitResult.GetActor();

			//Actor가 존재하고 IInteractable이 있다면
			if (HitActor && HitActor->Implements<UInteractable>())
			{
				//캐스팅
				IInteractable* Interactable = Cast<IInteractable>(HitActor);

				//성공했다면 상호작용
				if (Interactable)
				{
					//현재 Player
					Interactable->Interact(this);
				}
			}
		}
	}
//게터 현재 체력
float APlayerCharacter::GetCurrentHP()const
{
	return CurrentHP;
}
//게터 최대 체력
float APlayerCharacter::GetMaxHP()const
{
	return MaxHP;
}
//게터 현재마나
float APlayerCharacter::GetCurrentMana()const
{
	return CurrentMana;
}
//게터 최대마나
float APlayerCharacter::GetMaxMana()const
{
	return MaxMana;
}
//마나 소모함수
bool APlayerCharacter::ConsumeMana()
{
	//현재 마나가 0이하라면
	if (CurrentMana <= 0)
	{
		//함수실행되지않음
		UE_LOG(LogTemp, Warning, TEXT("Mana Emty!"));

		return false;
	}
	//현재마나가0이상이면 마나1소모
	CurrentMana--;
	UE_LOG(LogTemp, Warning, TEXT("Current Mana: %d / %d"),CurrentMana, MaxMana);

	//마나가 변경됫다고 알림
	OnManaChanged.Broadcast(CurrentMana, MaxMana);

	return true;
}
// 현재  장전이 가능한지 확인
void APlayerCharacter::ReloadMana()
{
	// 장전하고잇지않다면
	if (bIsReloading)
	{
		return;
	}
	//현재 마나가 최대마나보다 크거나 같다면
	if (CurrentMana >= MaxMana)
	{
		return;
	}
	//장전상태 트루로 변경
	bIsReloading = true;

	//재장전 몽타주
	if (ReloadMontage)
	{
		PlayAnimMontage(ReloadMontage);
	}

	PlayReloadSound();



	////2초후에 장전함수 실행
	//GetWorldTimerManager().SetTimer(
	//	ReloadTimerHandle,
	//	this,
	//	&APlayerCharacter::FinishReload,
	//	ReloadTime,
	//	false
	//);
}
//실제 장전 함수 
void APlayerCharacter::FinishReload()
{
	//마나 완충
	CurrentMana = MaxMana;
	//장전상태 종료
	bIsReloading = false;
	//마나가 변경됫다는 알림
	OnManaChanged.Broadcast(CurrentMana, MaxMana);

	UE_LOG(LogTemp, Warning, TEXT("Reload Complete! Mana: %d / &d"), CurrentMana, MaxMana);
}