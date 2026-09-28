#include "Elevator.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"

AElevator::AElevator()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root 생성
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 엘리베이터 메쉬
	ElevatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorMesh"));
	ElevatorMesh->SetupAttachment(SceneRoot);

	// 충돌 영역
	ElevatorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ElevatorCollision"));
	ElevatorCollision->SetupAttachment(SceneRoot);

	ElevatorCollision->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));

	ElevatorCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ElevatorCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ElevatorCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ElevatorCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AElevator::Interact(AActor* Interactor)
{
	// 이미 한 번 작동한 엘리베이터면 다시 사용 불가
	if (bActivated)
	{
		return;
	}

	// 상호작용한 Actor가 플레이어인지 확인
	APlayerCharacter* Player = Cast<APlayerCharacter>(Interactor);

	if (!Player)
	{
		return;
	}

	// 이미 이동 예약 중이거나 이동 중이면 사용 불가
	if (bIsMoving ||
		GetWorldTimerManager().IsTimerActive(ElevatorMoveTimerHandle))
	{
		return;
	}

	// 플레이어의 인벤토리 찾기
	UInventoryComponent* Inventory =
		Player->FindComponentByClass<UInventoryComponent>();

	if (!Inventory)
	{
		return;
	}

	const FName BossKey(TEXT("BossKey"));

	// 보스 열쇠가 없으면 작동하지 않음
	if (!Inventory->HasEnoughItem(BossKey, 1))
	{
		return;
	}

	// 보스 열쇠 1개 사용
	if (!Inventory->RemoveItems(BossKey, 1))
	{
		return;
	}

	// ★ 여기서부터 엘리베이터 사용 완료 상태
	bActivated = true;

	// 엘리베이터 작동
	MoveUp();
}

void AElevator::BeginPlay()
{
	Super::BeginPlay();

	// 엘리베이터가 처음 배치된 위치를 시작 위치로 저장
	StartLocation = GetActorLocation();

	// 위쪽 4500만큼 이동할 위치를 목표 위치로 설정
	TargetLocation = StartLocation + FVector(0.0f, 0.0f, 4500.0f);
}

void AElevator::MoveUp()
{
	//이미 이동중이거나 타이머 실행중이면 다시 실행 안함
	if (bIsMoving || GetWorldTimerManager().IsTimerActive(ElevatorMoveTimerHandle))
	{
		return;
	}

	// 5초 후 실제 상승 시작
	GetWorldTimerManager().SetTimer(
		ElevatorMoveTimerHandle,
		this,
		&AElevator::StartElevatorMovement,
		5.0f,
		false
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== ELEVATOR DEPARTS IN 5 SECONDS =====")
	);
}

void AElevator::StartElevatorMovement()
{
	bIsMoving = true;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== ELEVATOR MOVEMENT START =====")
	);
}

void AElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();

	const FVector NewLocation = FMath::VInterpConstantTo(
		CurrentLocation,
		TargetLocation,
		DeltaTime,
		ElevatorMoveSpeed
	);

	SetActorLocation(NewLocation);

	// 목표 위치에 도착하면 이동 종료
	if (NewLocation.Equals(TargetLocation, 1.0f))
	{
		SetActorLocation(TargetLocation);
		bIsMoving = false;
	}
}