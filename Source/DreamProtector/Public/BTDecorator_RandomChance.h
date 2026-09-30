#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RandomChance.generated.h"

UCLASS()
class DREAMPROTECTOR_API UBTDecorator_RandomChance : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_RandomChance();

	UPROPERTY(EditAnywhere, Category = "Random")
	float ChanceToExecute = 0.5f;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};