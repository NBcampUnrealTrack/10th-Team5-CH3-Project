#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bed.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

//스트레스가 바꼇을떄 알려주는 딜리게이트 매크로
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnStressChanged,
	int32, CurrentStress,
	int32, MaxStress
	);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBedAttacked);

UCLASS()
class DREAMPROTECTOR_API ABed : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ABed();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BedMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BedCollision;
	UPROPERTY(BluePrintAssignable, Category = "Stress")
	FOnStressChanged OnStressChganged;
	UFUNCTION(BlueprintPure, Category = "Stress")
	int32 GetCurrentStress() const;

	UFUNCTION(BlueprintPure, Category = "Stress")
	int32 GetMaxStress() const;
	UPROPERTY(BlueprintAssignable, Category = "Stress")
	FOnBedAttacked OnBedAttacked;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stress")
	int32 MaxStress = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stress")
	int32 CurrentStress = 0;

	virtual void BeginPlay() override;


public:	
	
	UFUNCTION(BlueprintCallable, Category = "Stress")
	void IncreaseStress(int32 amount);
	UFUNCTION(BlueprintCallable, Category = "Stress")
	void DecreaseStress(int32 amount);

};
