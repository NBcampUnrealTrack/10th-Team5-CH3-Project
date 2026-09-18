#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Interactable.h"
#include "WeaponBase.h"
#include "StaffBase.h"
#include "InventoryComponent.h"
#include "ProjectilePoolComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Animation/AnimInstance.h"
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

	ApplyControlMode(CurrentControlMode);

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

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInputComponent)
	{
		return;
	}

	// V키 카메라 모드 변경
	if (ChangeControlModeAction)
	{
		EnhancedInputComponent->BindAction(
			ChangeControlModeAction,
			ETriggerEvent::Started,
			this,
			&APlayerCharacter::ChangeControlMode
		);
	}
}

//실제 플레이어 이동함수
void APlayerCharacter::Move(const FVector2D& MovementVector)
{
	if (!Controller)
	{
		return;
	}

	//카메라가 바라보는 방향
	FRotator ControlRotation = Controller->GetControlRotation();

	//위/아래 Pitch는 이동 방향에 사용하지 않음
	FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	//카메라 기준 전방 / 우측 방향
	FVector ForwardDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	FVector RightDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	//입력 기준
	AddMovementInput(
		ForwardDirection,
		MovementVector.X
	);

	AddMovementInput(
		RightDirection,
		MovementVector.Y
	);
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
	PlayAttackSound();
}
//ABP
void APlayerCharacter::EndAttackAnimation()
{
	bIsAttacking = false;

	// 현재 카메라 모드의 회전 설정으로 복귀
	ApplyControlMode(CurrentControlMode);
}

void APlayerCharacter::StartAttack()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("StartAttack / Can:%d Attack:%d Reload:%d"),
		bCanAttack,
		bIsAttacking,
		bIsReloading
	);


	// 쿨타임 중이면 공격 불가
	if (!bCanAttack)
	{
		return;
	}

	// 이전 공격 모션이 아직 끝나지 않았다면 공격 불가
	if (bIsAttacking || bIsAutoFiring)
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

	// 공격 중에는 카메라가 보는 방향으로 캐릭터 고정
	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

	// 공격 몽타주 재생
	if (AttackMontage)
	{
		float MontageDuration = PlayAnimMontage(AttackMontage);
		
		if (MontageDuration > 0.0f)
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;

				EndDelegate.BindUObject(
					this,
					&APlayerCharacter::OnAttackMontageEnded
				);

				AnimInstance->Montage_SetEndDelegate(
					EndDelegate,
					AttackMontage
				);
			}
		}
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
	// 단발 공격 몽타주에서 연사 몽타주로 전환
	bIsAutoFiring = true;
	bIsAttacking = true;

	// 연사 중에는 카메라 바라보는 방향으로 몸 고정
	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

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
	PlayAttackSound();
}

void APlayerCharacter::StopAttack()
{
	// 연사 시작 대기 취소
	GetWorldTimerManager().ClearTimer(AutoFireStartTimerHandle);

	// 연사 종료
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	// 실제 연사 상태였다면
	// Attack01의 EndAttack Notify를 못 거쳤을 수 있으므로 직접 종료
	if (bIsAutoFiring && AutoFireMontage)
	{
		StopAnimMontage(AutoFireMontage);
	}

	bIsAutoFiring = false;
	bIsAttacking = false;


	// 현재 카메라 모드 회전 설정으로 복귀
	ApplyControlMode(CurrentControlMode);

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

	// 현재 무기의 장전 이펙트 실행
	if (AStaffBase* Staff = Cast<AStaffBase>(CurrentWeapon))
	{
		Staff->PlayReloadVFX();
	}
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


void APlayerCharacter::ChangeControlMode()
{
	switch (CurrentControlMode)
	{
	case EPlayControlMode::ThirdPerson:
		ApplyControlMode(EPlayControlMode::FirstPerson);
		break;

	case EPlayControlMode::FirstPerson:
		ApplyControlMode(EPlayControlMode::Shoulder);
		break;

	case EPlayControlMode::Shoulder:
		ApplyControlMode(EPlayControlMode::ThirdPerson);
		break;

	default:
		ApplyControlMode(EPlayControlMode::ThirdPerson);
		break;
	}
}

void APlayerCharacter::ApplyControlMode(EPlayControlMode NewControlMode)
{
	// 필요한 컴포넌트 확인
	UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement();

	if (!MovementComponent || !SpringArm || !Camera)
	{
		return;
	}

	// 현재 모드 변경
	CurrentControlMode = NewControlMode;

	switch (CurrentControlMode)
	{

		// 3인칭
	case EPlayControlMode::ThirdPerson:
	{
		// TopDown에서 사용했던 절대 회전 해제
		SpringArm->SetUsingAbsoluteRotation(false);

		// 마우스 회전을 카메라가 따라감
		SpringArm->bUsePawnControlRotation = true;

		// 벽 충돌 활성화
		SpringArm->bDoCollisionTest = true;

		// 기본 TPS 거리
		SpringArm->TargetArmLength = 400.0f;

		// 캐릭터보다 조금 높은 위치
		SpringArm->TargetOffset =
			FVector(0.0f, 0.0f, 60.0f);

		//기존에 사용하던 TPS Offset
		SpringArm->SocketOffset =
			FVector(0.0f, 0.0f, 50.0f);

		Camera->SetFieldOfView(90.0f);

		// 카메라를 돌려도 캐릭터가 바로 따라 돌지 않음
		bUseControllerRotationYaw = false;

		// 이동 방향으로 몸 회전
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->bUseControllerDesiredRotation = false;

		// 1인칭에서 숨겼던 Mesh 다시 표시
		GetMesh()->SetOwnerNoSee(false);

		break;
	}

	// 1인칭
	case EPlayControlMode::FirstPerson:
	{
		SpringArm->SetUsingAbsoluteRotation(false);

		SpringArm->bUsePawnControlRotation = true;

		// ArmLength 0이므로 충돌 검사
		SpringArm->bDoCollisionTest = false;

		// 캐릭터 위치까지 카메라 이동
		SpringArm->TargetArmLength = 0.0f;

		// 눈높이
		SpringArm->TargetOffset =
			FVector(
				0.0f,
				0.0f,
				BaseEyeHeight
			);

		SpringArm->SocketOffset =
			FVector::ZeroVector;

		Camera->SetFieldOfView(90.0f);

		// 마우스 Yaw를 캐릭터도 따라감
		bUseControllerRotationYaw = true;

		// 이동 방향 자동 회전 OFF
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;

		// 몸이 카메라를 가리지 않도록
		GetMesh()->SetOwnerNoSee(true);

		break;
	}

	// 숄더 / 조준
	case EPlayControlMode::Shoulder:
	{
		SpringArm->SetUsingAbsoluteRotation(false);

		SpringArm->bUsePawnControlRotation = true;

		SpringArm->bDoCollisionTest = true;

		// 일반 TPS보다 가까이
		SpringArm->TargetArmLength = 220.0f;

		SpringArm->TargetOffset =
			FVector(
				0.0f,
				0.0f,
				60.0f
			);

		// 오른쪽 어깨 방향
		SpringArm->SocketOffset =
			FVector(
				0.0f,
				90.0f,
				0.0f
			);

		//FOV 좁게 사용
		Camera->SetFieldOfView(80.0f);

		// 조준 방향으로 캐릭터 몸도 회전
		bUseControllerRotationYaw = true;

		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;

		GetMesh()->SetOwnerNoSee(false);

		break;
	}

	default:
		break;
	}
}

void APlayerCharacter::OnAttackMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted
)
{
	// 연사로 넘어간 상태가 아니라면 단발 공격 종료
	if (!bIsAutoFiring)
	{
		bIsAttacking = false;

		ApplyControlMode(CurrentControlMode);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Attack Montage Ended / Interrupted: %d"),
		bInterrupted
	);
}

bool APlayerCharacter::UseItem(FName ItemKey)
{
	return false;
}