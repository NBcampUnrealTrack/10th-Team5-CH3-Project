#include "ElevatorKey.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Elevator.h"

AElevatorKey::AElevatorKey()
{
	PrimaryActorTick.bCanEverTick = true;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	RootComponent = KeyMesh;

	KeyCollision = CreateDefaultSubobject<USphereComponent>(TEXT("KeyCollision"));
	KeyCollision->SetupAttachment(RootComponent);
	KeyCollision->SetSphereRadius(300.0f);

	KeyCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AElevatorKey::OnKeyOverlap
	);
}

void AElevatorKey::SetElevator(AElevator* InElevator)
{
	Elevator = InElevator;
}

void AElevatorKey::OnKeyOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== KEY OVERLAP EVENT FIRED =====")
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("KEY OVERLAP ACTOR: %s"),
		*GetNameSafe(OtherActor)
	);

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	if (!Elevator)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("===== ELEVATOR REFERENCE IS INVALID =====")
		);

		return;
	}


	UE_LOG(
		LogTemp,
		Warning,
		TEXT("===== ELEVATOR KEY ACQUIRED =====")
	);

	Destroy();
}

void AElevatorKey::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
}

void AElevatorKey::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// 처음 위치를 기준으로 부드럽게 위아래 이동
	FVector NewLocation = StartLocation;

	NewLocation.Z += FMath::Sin(
		RunningTime * FloatFrequency * 2.0f * PI
	) * FloatHeight;

	SetActorLocation(NewLocation);

	// 수직축을 중심으로 회전
	AddActorLocalRotation(
		FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f)
	);
}