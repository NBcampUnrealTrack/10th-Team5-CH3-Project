#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

class UbBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

UCLASS()
class DREAMPROTECTOR_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	// 생성자 생성
	AMonsterAIController();

	// 컨트롤러가 몬스터(Pawn)에 빙의 하면 BT 실행
	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	// 몬스터가 실행할 행동 트리 에셋(BT 에디터 연결)
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	void SetInitialTarget();
};
