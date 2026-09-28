#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossMeleeAttack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTTask_BossMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossMeleeAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
