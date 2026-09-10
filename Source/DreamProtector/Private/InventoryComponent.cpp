#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UInventoryComponent::AddItem(FName ItemKey)
{
	//전달받은 아이템 Key를 인벤토리 배열에 추가
	ItemKeys.Add(ItemKey);

	UE_LOG(LogTemp, Warning, TEXT("Item Added: %s"), *ItemKey.ToString());
}


