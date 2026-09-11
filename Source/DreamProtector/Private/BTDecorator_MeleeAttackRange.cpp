#include "BTDecorator_MeleeAttackRange.h"
#include "AIController.h"
#include "Melee.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_MeleeAttackRange::UBTDecorator_MeleeAttackRange()
{
	NodeName = TEXT("Melee: Attack Range");
}

bool UBTDecorator_MeleeAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	AMelee* Monster = Cast<AMelee>(AIController->GetPawn());
	if (!Monster) return false;

	// Blackboard의 TargetActor 값을 Monster의 Target에 동기화 (원거리와 같음)
	if (UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")))
	{
		if (AActor* TargetActor = Cast<AActor>(TargetObject))
		{
			Monster->SetTarget(TargetActor);
		}
	}

	// return Monster->IsTargetInMeleeRange();
	bool bResult = Monster->IsTargetInMeleeRange();
	UE_LOG(LogTemp, Log, TEXT("[Decorator] MeleeInAttackRange 체크 결과: %s"), bResult ? TEXT("true") : TEXT("false"));

	return bResult;

}
