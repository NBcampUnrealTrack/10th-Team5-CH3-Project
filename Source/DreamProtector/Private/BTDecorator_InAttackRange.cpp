#include "BTDecorator_InAttackRange.h"
#include "AIController.h"
// 해당 AttackRange가 FlyingRanged 전용 함수라 캐스팅
#include "FlyingRanged.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_InAttackRange::UBTDecorator_InAttackRange()
{
	// BT 에이터 노드 표시 이름
	NodeName = TEXT("InAttackRange");
}

bool UBTDecorator_InAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// BT를 돌리는 AIController 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	// 빙의된 Pawn을 FlyingRanged로 캐스팅
	AFlyingRanged* Monster = Cast<AFlyingRanged>(AIController->GetPawn());
	if (!Monster) return false;

	if (UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")))
	{
		if (AActor* TargetActor = Cast<AActor>(TargetObject))
		{
			Monster->SetTarget(TargetActor);
		}
	}

	// 만든 함수 호출, true / false 결과를 그대로 BT에 전달쓰~
	return Monster->IsTargetInAttackRange();
}
