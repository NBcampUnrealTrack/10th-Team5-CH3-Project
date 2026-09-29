#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class DREAMPROTECTOR_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnVolume();

	// 스폰박스 내부에서 랜덤한 스폰 위치를 반환
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomSpawnLocation() const;

protected:
	//몬스터가 스폰되는 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<class UBoxComponent> SpawnBox;

public:	

};
