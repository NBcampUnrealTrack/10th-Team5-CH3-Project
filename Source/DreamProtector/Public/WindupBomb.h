#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindupBomb.generated.h"

class USphereComponent;

UCLASS()
class DREAMPROTECTOR_API AWindupBomb : public AActor
{
    GENERATED_BODY()

public:
    AWindupBomb();

protected:
    virtual void BeginPlay() override;

    // 폭탄 외형
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
    TObjectPtr<UStaticMeshComponent> BombMesh;

    // 폭발 범위
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
    TObjectPtr<USphereComponent> ExplosionRange;

    // 설치 후 폭발까지 걸리는 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
    float ExplosionDelay = 2.0f;

    // 폭발 데미지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
    float ExplosionDamage = 100.0f;

    // 실제 폭발 처리
    void Explode();

private:
    FTimerHandle ExplosionTimerHandle;
};