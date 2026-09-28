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
}

void AElevator::Interact(AActor* Interactor)
{
	//상호작용한 Actor가 플레이어인지 확인 
	APlayerCharacter* Player = Cast<APlayerCharacter>(Interactor);

	if (!Player)
	{
		return;
	}

	//이미 이동예약이거나 이동중이면 추가 조작X
	if (bIsMoving || GetWorldTimerManager().IsTimerActive(ElevatorMoveTimerHandle))
	{
		return;
	}

	//플레이어가 가지고 있는 InventoryComponent 찾기
	UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();

	if (!Inventory)
	{
		return;
	}

	const FName BossKey(TEXT("BossKey"));

	//보스키 가지고 있는지 확인
	if (!Inventory->HasEnoughItem(BossKey, 1))
	{
		return;
	}

	//보스키 제거
	if (!Inventory->RemoveItems(BossKey, 1))
	{
		return;
	}

	//엘리베이터 작동
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