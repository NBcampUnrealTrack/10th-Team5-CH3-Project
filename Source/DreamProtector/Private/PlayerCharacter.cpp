#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Interactable.h"
#include "WeaponBase.h"
#include "StaffBase.h"
#include "InventoryComponent.h"
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
	//카메라 컴포넌트추가
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;


	//인벤토리 컴포넌트 추가
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}



//임시 테스트용
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	CurrentMana = MaxMana;
	
	AActor* FoundWeapon = UGameplayStatics::GetActorOfClass(GetWorld(), AStaffBase::StaticClass());

	CurrentWeapon = Cast<AWeaponBase>(FoundWeapon);

	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Found: %s"), *CurrentWeapon->GetName());
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

void APlayerCharacter::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Player Attack Called"));
	//리로드 중이면 공격 불가능
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Attack: Reloading"));
		return;
	}
	// 무기가 없다면 공격불가능
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentWeapon is nullptr"));
		return;
	}
	// 현재 마나가 없다면 공격불가능
	if (!ConsumeMana())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Enough Mana"));
		return;
	}
	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack??"));
		CurrentWeapon->Attack();
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
	UE_LOG(LogTemp, Warning, TEXT("Reload Start"));

	//2초후에 장전함수 실행
	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&APlayerCharacter::FinishReload,
		ReloadTime,
		false
	);
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