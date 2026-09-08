#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class DREAMPROTECTOR_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();

protected:
	//공격 데미지 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponBase")
	float Damage;
	//공격 속도 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponBase")
	float AttackInterval;
	//사거리 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponBase")
	float Range;

	//루트 컴포넌트 포인터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;
	//static Mesh Component 포인터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComp;

	//Attack 가상함수
	virtual void Attack();


};
