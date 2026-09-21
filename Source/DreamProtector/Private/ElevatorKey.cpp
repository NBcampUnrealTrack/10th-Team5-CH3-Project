#include "ElevatorKey.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Elevator.h"
#include "Kismet/GameplayStatics.h"

AElevatorKey::AElevatorKey()
{
	PrimaryActorTick.bCanEverTick = false;

	// 키의 실제 모습
	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	RootComponent = KeyMesh;

	// 플레이어가 키에 닿았는지 감지하는 영역
	KeyCollision = CreateDefaultSubobject<USphereComponent>(TEXT("KeyCollision"));
	KeyCollision->SetupAttachment(RootComponent);

	// 키를 중심으로 일정 범위 안에 들어오면 감지
	KeyCollision->SetSphereRadius(300.0f);

	// 오버랩 이벤트 연결
	KeyCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AElevatorKey::OnKeyOverlap
	);
}

void AElevatorKey::OnKeyOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("===== KEY OVERLAP EVENT FIRED ====="));

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("KEY OVERLAP ACTOR: %s"),
		*GetNameSafe(OtherActor)
	);
	// 플레이어가 아니면 무시
	if (!OtherActor || !OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	// 현재 레벨의 엘리베이터 찾기
	if (AElevator* Elevator = Cast<AElevator>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(),
			AElevator::StaticClass()
		)
	))
	{
		// 엘리베이터에 키 획득 상태 전달
		Elevator->SetHasOperatingKey(true);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("===== ELEVATOR KEY ACQUIRED =====")
		);
	}

	// 획득한 키 제거
	Destroy();
}
