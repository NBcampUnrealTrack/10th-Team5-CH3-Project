#include "BTDecorator_MeleeCanAttack.h"
#include "AIController.h"
#include "Melee.h"

UBTDecorator_MeleeCanAttack::UBTDecorator_MeleeCanAttack()
{
	NodeName = TEXT("Melee: Can Attack");
}

bool UBTDecorator_MeleeCanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	AMelee* Monster = Cast<AMelee>(AIController->GetPawn());
	if (!Monster) return false;

	// Target을 사용하지 않는 함수라서 동기화 코드 X
	return Monster->CanAttack();
}
