#include "BossAIController.h"
#include "BehaviorTree/BehaviorTree.h"

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ABossAIController::StartBossBattle()
{
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}