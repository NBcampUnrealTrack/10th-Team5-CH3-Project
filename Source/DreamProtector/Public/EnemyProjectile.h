#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyProjectile.generated.h"

// 헤더에서는 클래스가 있다는 것만 미리 알려줌
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class DREAMPROTECTOR_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AEnemyProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 투사체의 충돌 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	// 실제로 화면에 보이는 투사체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	// 투사체 이동 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 플레이어에게 줄 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Damage = 10.0f;
	// 투사체가 다른 Actor와 겹쳤을 때 호출
};