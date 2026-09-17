#include "BTDecorator_ChestCanAttack.h"
#include "AIController.h"
#include "Melee.h"

UBTDecorator_ChestCanAttack::UBTDecorator_ChestCanAttack()
{
	NodeName = TEXT("ChestCanAttack");
}

bool UBTDecorator_ChestCanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	AMelee* Monster = Cast<AMelee>(AIController->GetPawn());
	if (!Monster) return false;

	return Monster->CanAttack();
}

