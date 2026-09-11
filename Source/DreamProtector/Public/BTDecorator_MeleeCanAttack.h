#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MeleeCanAttack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_MeleeCanAttack : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_MeleeCanAttack();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
