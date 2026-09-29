#include "BTTask_BossMeleeAttack.h"
#include "AIController.h"
#include "BossMonster.h"

UBTTask_BossMeleeAttack::UBTTask_BossMeleeAttack()
{
	NodeName = TEXT("Melee Attack");
}

EBTNodeResult::Type UBTTask_BossMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// 범위 밖이면 즉시 실패 -> Selector가 다음(원거리)으로 넘어감
	return Boss->TryStartMeleeAttack() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}