#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MyBTTask_MoveTowardsTarget.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTTask_MoveTowardsTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	// 생성자 Task 이름 지정
	UBTTask_MoveTowardsTarget();

	//BT가 해당 Task 노드를 실행할 때마다 매 프레임 호출 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
