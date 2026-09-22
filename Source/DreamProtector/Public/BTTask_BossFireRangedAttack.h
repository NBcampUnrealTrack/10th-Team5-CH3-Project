#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossFireRangedAttack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTTask_BossFireRangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_BossFireRangedAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};