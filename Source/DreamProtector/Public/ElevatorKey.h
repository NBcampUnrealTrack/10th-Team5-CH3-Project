#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElevatorKey.generated.h"

UCLASS()
class DREAMPROTECTOR_API AElevatorKey : public AActor
{
	GENERATED_BODY()

public:
	AElevatorKey();

protected:
	// 키의 실제 모습
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator Key")
	TObjectPtr<class UStaticMeshComponent> KeyMesh;

	// 플레이어가 키에 닿았는지 감지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator Key")
	TObjectPtr<class USphereComponent> KeyCollision;

	// 키를 획득했을 때 처리
	UFUNCTION()
	void OnKeyOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};