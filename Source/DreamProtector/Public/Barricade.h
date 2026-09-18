#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Barricade.generated.h"

class UStaticMeshComponent;

UCLASS()
class DREAMPROTECTOR_API ABarricade : public AActor
{
    GENERATED_BODY()

public:
    ABarricade();

protected:
    virtual void BeginPlay() override;

    // 바리케이드 외형 + 충돌
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade")
    TObjectPtr<UStaticMeshComponent> BarricadeMesh;

    // 설치 후 유지되는 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barricade")
    float Duration = 30.0f;
};