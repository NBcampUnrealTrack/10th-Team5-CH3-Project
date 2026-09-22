#include "BTTask_BossFireRangedAttack.h"
#include "AIController.h"
#include "BossMonster.h"

UBTTask_BossFireRangedAttack::UBTTask_BossFireRangedAttack()
{
	NodeName = TEXT("BossFireRangedAttack");
}

EBTNodeResult::Type UBTTask_BossFireRangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	Boss->FireRangedAttack();

	return EBTNodeResult::Succeeded;
}
