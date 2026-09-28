#include "BTTask_BossGroundBlast.h"
#include "AIController.h"
#include "BossMonster.h"

UBTTask_BossGroundBlast::UBTTask_BossGroundBlast()
{
	NodeName = TEXT("Ground Blast");
}

EBTNodeResult::Type UBTTask_BossGroundBlast::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABossMonster* Boss = Cast<ABossMonster>(AIController->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	// 이미 장판이 진행 중이면 false -> 실패로 넘어가 중복 시전을 막음
	return Boss->StartGroundBlast() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
