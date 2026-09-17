#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ChestCanAttack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_ChestCanAttack : public UBTDecorator
{
	GENERATED_BODY()
	

public:
	UBTDecorator_ChestCanAttack();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
