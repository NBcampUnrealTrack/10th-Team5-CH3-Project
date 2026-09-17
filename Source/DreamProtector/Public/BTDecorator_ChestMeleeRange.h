#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ChestMeleeRange.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_ChestMeleeRange : public UBTDecorator
{
	GENERATED_BODY()


public:
	UBTDecorator_ChestMeleeRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
