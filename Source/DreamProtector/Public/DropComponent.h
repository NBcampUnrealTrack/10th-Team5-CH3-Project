#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DropComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAMPROTECTOR_API UDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDropComponent();

	//몬스터가 죽었을때 호출
	//등록된 아이템 중 하나를 확률에 따라 드롭
	UFUNCTION(BlueprintCallable, Category = "Drop")
	void DropItem();

protected:
	virtual void BeginPlay() override;

	//몬스터가 드롭할 수 있는 아이템 클래스 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	TArray<TSubclassOf<AActor>>DropItemClasses;

	//드롭 확률 (1.0 == 100%, 0.3 == 30%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.3f;
public:	


};
