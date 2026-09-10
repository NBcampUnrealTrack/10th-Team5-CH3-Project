#pragma once

#include "CoreMinimal.h"
#include "BaseMonster.h"
#include "Components/SphereComponent.h"
#include "Suicide.generated.h"


UCLASS()
class DREAMPROTECTOR_API ASuicide : public ABaseMonster
{
	GENERATED_BODY()

public:

	ASuicide();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suicide")
	TObjectPtr<USphereComponent> ExplosionCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suicide")
	float ExplosionDamage = 50.0f;

	virtual void Attack_Implementation() override;
};