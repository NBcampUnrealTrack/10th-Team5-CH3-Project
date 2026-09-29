#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Elevator.generated.h"


UCLASS()
class DREAMPROTECTOR_API AElevator : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AElevator();

	// 플레이어가 엘리베이터와 상호작용했을 때 호출
	virtual void Interact(AActor* Interactor) override;

	// 엘리베이터를 위로 이동
	UFUNCTION(BlueprintCallable, Category = "Elevator")
	void MoveUp();

	FTimerHandle ElevatorMoveTimerHandle;

	// 5초 대기가 끝난 뒤 실제 이동을 시작한다.
	void StartElevatorMovement();
	bool IsActivated() const { return bActivated; }
	virtual void Tick(float DeltaTime) override;
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
	TObjectPtr<UStaticMeshComponent> ElevatorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
	TObjectPtr<UBoxComponent> ElevatorCollision;
	// 이동 시작 위치
	FVector StartLocation;
	// 이동 목표 위치
	FVector TargetLocation;

	bool bIsMoving = false;

	float ElevatorMoveSpeed = 500.0f;

	virtual void BeginPlay() override;

private:
	bool bActivated = false;
};