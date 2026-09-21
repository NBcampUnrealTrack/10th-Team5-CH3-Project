#include "Elevator.h"
#include "Components/StaticMeshComponent.h"

AElevator::AElevator()
{
	PrimaryActorTick.bCanEverTick = true;

	// 엘리베이터 메쉬 컴포넌트 생성
	ElevatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorMesh"));

	// 엘리베이터 메쉬를 루트 컴포넌트로 설정
	RootComponent = ElevatorMesh;

	// 플레이어가 엘리베이터에 올라왔는지 감지하는 영역
	ElevatorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ElevatorCollision"));
	ElevatorCollision->SetupAttachment(RootComponent);

	// 엘리베이터 바닥보다 조금 넓은 감지 영역
	ElevatorCollision->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));

	// 플레이어가 들어오면 Overlap 이벤트 발생
	ElevatorCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ElevatorCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ElevatorCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Overlap 이벤트 연결
	ElevatorCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AElevator::OnElevatorOverlap
	);
}

void AElevator::BeginPlay()
{
	Super::BeginPlay();

	// 엘리베이터가 처음 배치된 위치를 시작 위치로 저장
	StartLocation = GetActorLocation();

	// 일단 테스트용으로 위쪽 500만큼 이동할 위치를 목표 위치로 설정
	TargetLocation = StartLocation + FVector(0.0f, 0.0f, 4500.0f);
}

void AElevator::SetHasOperatingKey(bool bHasKey)
{
	bHasOperatingKey = bHasKey;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Elevator Operating Key: %s"),
		bHasOperatingKey ? TEXT("YES") : TEXT("NO")
	);
}

void AElevator::OnElevatorOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 플레이어가 아니면 무시
	if (!OtherActor || !OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	// 아직 엘리베이터 작동 키가 없다면 이동하지 않음
	if (!bHasOperatingKey)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Elevator requires the operating key.")
		);

		return;
	}

	// 키가 있다면 엘리베이터 이동
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== ELEVATOR MOVE UP =====")
	);

	MoveUp();
}

void AElevator::MoveUp()
{
	if (bIsMoving)
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