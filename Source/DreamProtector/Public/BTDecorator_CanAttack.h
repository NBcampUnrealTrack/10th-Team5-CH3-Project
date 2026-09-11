#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanAttack.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_CanAttack : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	// 생성자
	UBTDecorator_CanAttack();

	// BT가 이 조건을 체크할 때 호출 함수. true / false 사용
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};