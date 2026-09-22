#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Elevator.generated.h"

UCLASS()
class DREAMPROTECTOR_API AElevator : public AActor
{
	GENERATED_BODY()

public:
	AElevator();
	// 엘리베이터를 위로 이동
	UFUNCTION(BlueprintCallable, Category = "Elevator")
	void MoveUp();

	FTimerHandle ElevatorMoveTimerHandle;

	void StartElevatorMovement();

	// 엘리베이터 작동 키를 획득했는지 설정
	void SetHasOperatingKey(bool bHasKey);

	// 엘리베이터 작동 키를 가지고 있는지 확인
	bool HasOperatingKey() const { return bHasOperatingKey; }

	// 이동 시작 위치
	FVector StartLocation;

	// 이동 목표 위치
	FVector TargetLocation;

	virtual void Tick(float DeltaTime) override;
protected:
	// 엘리베이터 바닥
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
	TObjectPtr<class UStaticMeshComponent> ElevatorMesh;

	// 플레이어가 엘리베이터 키를 가지고 있는지
	UPROPERTY()
	bool bHasOperatingKey = false;

	// 플레이어가 엘리베이터에 올라왔는지 감지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
	TObjectPtr<UBoxComponent> ElevatorCollision;

	// 플레이어가 엘리베이터에 올라왔을 때 호출
	UFUNCTION()
	void OnElevatorOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	// 엘리베이터 이동 처리
	void UpdateElevatorMovement(float DeltaTime);

	// 엘리베이터가 이동 중인지
	bool bIsMoving = false;

	// 엘리베이터 이동 속도
	float ElevatorMoveSpeed = 500.0f;

	virtual void BeginPlay() override;
};