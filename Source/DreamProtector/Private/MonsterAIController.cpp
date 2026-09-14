#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet//GameplayStatics.h"
#include "BaseMonster.h"

AMonsterAIController::AMonsterAIController()
{
	// 현재 초기화 할 내용 없음
}

// Pawn에 빙의되면 바로 자동 호출
void AMonsterAIController::OnPossess(APawn* InPawn)
{
	// 부모(AAIController)의 빙의 처리 먼저 실행 (필수!!!)
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
		
		// 살짝 지연 후 타겟 설정
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AMonsterAIController::SetInitialTarget, 0.2f, false);
		
	}
}

void AMonsterAIController::SetInitialTarget()
{
	// 이미 Target이 설정되있으면 덮어쓰지 않음
	if (ABaseMonster* Monster = Cast<ABaseMonster>(GetPawn()))
	{
		// GetTarget()이라는 getter가 필요
		if (Monster->GetTarget() != nullptr)
		{
			// 이미 타겟이 있으니까 스킵
			return;
		}
	}
	// BlackBoard에 TargetActor 채우기 (임시: 0번 플레이어 자동 타겟팅)
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		// BT 실행, Blackboard 자동 생성 및 연결
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);
	}
}
