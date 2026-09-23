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
};