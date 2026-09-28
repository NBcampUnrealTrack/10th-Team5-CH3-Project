#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElevatorKey.generated.h"

class AElevator;

UCLASS()
class DREAMPROTECTOR_API AElevatorKey : public AActor
{
	GENERATED_BODY()

public:

	AElevatorKey();

	void SetElevator(AElevator* InElevator);

	virtual void Tick(float DeltaTime) override;
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator Key")
	TObjectPtr<class UStaticMeshComponent> KeyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator Key")
	TObjectPtr<class USphereComponent> KeyCollision;

	UPROPERTY()
	TObjectPtr<AElevator> Elevator;

	UFUNCTION()
	void OnKeyOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	virtual void BeginPlay() override;

	// 처음 배치된 위치
	FVector StartLocation;

	// 움직임 계산에 사용할 누적 시간
	float RunningTime = 0.0f;

	// 위아래로 움직이는 거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Motion")
	float FloatHeight = 10.0f;

	// 1초 동안 위아래로 왕복하는 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Motion")
	float FloatFrequency = 0.5f;

	// 1초 동안 회전하는 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Motion")
	float RotationSpeed = 60.0f;
};