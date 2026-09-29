#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileBase.generated.h"

class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;

UCLASS()
class DREAMPROTECTOR_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

protected:
	//충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComp;
	//투사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;
	//타이머 추가
	FTimerHandle DeactivateTimerHandle;
	//투사체 회전속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FRotator SpinSpeed = FRotator(0.0f, 0.0f, 720.0f);

	//투사체가 다른 액터와 Overlap 되었을 때 호출
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Impact")
	UNiagaraSystem* ImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Impact")
	USoundBase* ImpactSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Impact")
	USoundAttenuation* ImpactAttenuation;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ActivateProjectile(
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation
	);
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void DeactivateProjectile();
	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	//효과 재생용 함수
	void PlayImpactEffect(
		const FVector& Location,
		const FVector& Normal
	);
	

};
