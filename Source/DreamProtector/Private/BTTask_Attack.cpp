#include "BTTask_Attack.h"
#include "AIController.h"
// AFlyingRanged 원거리 가 아닌 부모(BaseMoster)를 공용으로 쓰기 위해
#include "BaseMonster.h"

UBTTask_Attack::UBTTask_Attack()
{
	// BT 에이터에서 표시될 노드 이름
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 해당 BT를 돌리는 AIController 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	// AFlyingRanged가 아니라 ABaseMonster로 캐스팅!
	// -> 원거리 / 근접 / 자폭병 등 몬스터가 빙의돼 있어도 다 통과!
	ABaseMonster* Monster = Cast<ABaseMonster>(AIController->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	UE_LOG(LogTemp, Error, TEXT("[BTTask_Attack] Monster 클래스 이름: %s"), *Monster->GetClass()->GetName());

	// 다형성 때문에 실제 몬스터 종류에 맞는 Attack_Imlementation() 자동 실행
	Monster->Attack();

	// Task 성공 처리~
	return EBTNodeResult::Succeeded;
}
