#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAMPROTECTOR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	//전달받은 ItemKey를 인벤토리에 추가하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemKey);

	//Getter
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FName>& GetItemKeys() const
	{
		return ItemKeys;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemKey) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItems(FName ItemKey, int32 Count);

protected:
	//인벤토리에 들어온 아이템의 Key들을 저장하는 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FName> ItemKeys;
};
