#pragma once

#include "CoreMinimal.h"
#include "BaseMonster.h"
#include "Components/SphereComponent.h"
#include "Melee.generated.h"

UCLASS()
class DREAMPROTECTOR_API AMelee : public ABaseMonster
{
	GENERATED_BODY()
	
public:

	AMelee();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee")
	TObjectPtr<USphereComponent> AttackCollision;

	virtual void Attack_Implementation() override;
};
