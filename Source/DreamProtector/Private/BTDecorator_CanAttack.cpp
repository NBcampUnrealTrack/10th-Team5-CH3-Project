#include "BTDecorator_CanAttack.h"
#include "AIController.h"
// 해당 AttackRange가 FlyingRanged 전용 함수라 캐스팅
#include "FlyingRanged.h"

UBTDecorator_CanAttack::UBTDecorator_CanAttack()
{
	// BT 에이터 노드 표시 이름
	NodeName = TEXT("CanAttack");
}

bool UBTDecorator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// BT를 돌리는 AIController 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	// 빙의된 Pawn을 FlyingRanged로 캐스팅
	AFlyingRanged* Monster = Cast<AFlyingRanged>(AIController->GetPawn());
	if (!Monster) return false;

	// 쿨타임 지났는지 체크~
	return Monster->CanAttack();
}
