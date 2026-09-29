#include "BTDecorator_ChestMeleeRange.h"
#include "AIController.h"
#include "Melee.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_ChestMeleeRange::UBTDecorator_ChestMeleeRange()
{
	NodeName = TEXT("ChestMeleeRange");
}

bool UBTDecorator_ChestMeleeRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	AMelee* Monster = Cast<AMelee>(AIController->GetPawn());
	if (!Monster) return false;

	if (UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")))
	{
		if (AActor* TargetActor = Cast<AActor>(TargetObject))
		{
			Monster->SetTarget(TargetActor);
		}
	}
	return Monster->IsTargetInMeleeRange();
}