#include "BTTask_MoveTowardsTarget.h"
#include "AIController.h"
#include "FlyingRanged.h"

UBTTask_MoveTowardsTarget::UBTTask_MoveTowardsTarget()
{
	// BT 에디터에서 노드 표시될 이름
	NodeName = TEXT("MoveTowardsTarget");
}

EBTNodeResult::Type UBTTask_MoveTowardsTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 해당 BT 돌리고 있는 AIController 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	// 없으면 실패당~
	if (!AIController) return EBTNodeResult::Failed;

	// 빙의된 Pawn을 FlyingRanged로 캐스팅
	AFlyingRanged* Monster = Cast<AFlyingRanged>(AIController->GetPawn());
	// 캐스팅 없으면 (다른 몬스터가 없을 경우) 실패당~ 
	if(!Monster) return EBTNodeResult::Failed;

	// 우리가 이미 만들어둔 함수 (실제 이동 로직) 실행
	Monster->MoveTowardsTarget();
	// Task가 성공적으로 끝났다고 BT에 알림
	return EBTNodeResult::Succeeded;
}
