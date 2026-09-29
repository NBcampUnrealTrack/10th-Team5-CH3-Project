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

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void PlayCastVFX(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void PlayReloadVFX();



	// 공격키를 뗐을 때 VFX 생성 가능 상태로 초기화
	void ResetCastVFX();
protected:
	// 현재 공격 입력에서 마법진을 이미 생성했는지
	bool bCastVFXPlayed = false;

	virtual void Attack() override;


	//ProjectileBase를 상속받은 클래스 종류 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff")
	USceneComponent* MuzzlePoint;
};