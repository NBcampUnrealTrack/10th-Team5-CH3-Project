#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossGroundBlast.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTTask_BossGroundBlast : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossGroundBlast();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};