#include "ElevatorKey.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Elevator.h"

AElevatorKey::AElevatorKey()
{
	PrimaryActorTick.bCanEverTick = false;

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