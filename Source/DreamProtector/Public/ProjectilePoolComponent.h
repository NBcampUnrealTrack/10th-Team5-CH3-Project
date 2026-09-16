#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectilePoolComponent.generated.h"

class AProjectileBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DREAMPROTECTOR_API UProjectilePoolComponent : public UActorComponent
{
  GENERATED_BODY()

public:
  UProjectilePoolComponent();

protected:
  virtual void BeginPlay() override;

public:
  // 풀에서 사용할 Projectile 클래스
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Pool")
  TSubclassOf<AProjectileBase> ProjectileClass;

  // 처음에 미리 생성해둘 구미베어 개수 (일단 20개 추후 공속이 빨라지면 생성 늘려야함)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Pool")
  int32 PoolSize = 20;

  // 현재 사용하지 않는 Projectile 하나를 찾아서 반환하는 함수
  UFUNCTION(BlueprintCallable, Category = "Projectile Pool")
  AProjectileBase* GetProjectile();

protected:
  // 실제로 미리 생성된 Projectile들을 저장하는 배열
  UPROPERTY()
  TArray<AProjectileBase*> ProjectilePool;

};