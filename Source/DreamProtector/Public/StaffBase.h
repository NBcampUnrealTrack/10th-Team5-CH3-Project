#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ProjectileBase.h"
#include "StaffBase.generated.h"


UCLASS()
class DREAMPROTECTOR_API AStaffBase : public AWeaponBase
{
	GENERATED_BODY()
	

public:
	AStaffBase();

protected:

	virtual void Attack() override;

	void BeginPlay() override;

	//ProjectileBase를 상속받은 클래스 종류 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff")
	TSubclassOf<AProjectileBase> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff")
	USceneComponent* MuzzlePoint;

};
