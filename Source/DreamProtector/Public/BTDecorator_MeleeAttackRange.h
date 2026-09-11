#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MeleeAttackRange.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_MeleeAttackRange : public UBTDecorator
{
	GENERATED_BODY()
	

public:
	UBTDecorator_MeleeAttackRange();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
