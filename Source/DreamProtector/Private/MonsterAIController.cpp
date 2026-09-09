#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AMonsterAIController::AMonsterAIController()
{
	// 현재 초기화 할 내용 없음
}

// Pawn에 빙의되면 바로 자동 호출
void AMonsterAIController::OnPossess(APawn* InPawn)
{
	// 부모(AAIController)의 빙의 처리 먼저 실행 (필수!!!)
	Super::OnPossess(InPawn);

	// BT 에셋이 연결 확인(안 되있으면 nullptr로 에러)
	if (BehaviorTreeAsset)
	{
		// BT 실행, Blackboard 자동 생성 및 연결
		RunBehaviorTree(BehaviorTreeAsset);
	}
}