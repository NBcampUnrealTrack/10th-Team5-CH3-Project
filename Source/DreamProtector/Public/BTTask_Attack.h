#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	// 생성자 
	UBTTask_Attack();

	// BT가 이 노드를 실행하면 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
