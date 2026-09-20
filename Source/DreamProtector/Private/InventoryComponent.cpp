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

    OnInventoryChanged.Broadcast();
}


int32 UInventoryComponent::GetItemCount(FName ItemKey)const
{
	int32 Count = 0;


	for (const FName& Key : ItemKeys)
	{
		if (Key == ItemKey)
		{
			Count++;
		}
	}

	return Count;
}

bool UInventoryComponent::RemoveItems(FName ItemKey, int32 Count)
{
    // 먼저 내가 이 아이템을 충분히 가지고 있는지 확인
    if (GetItemCount(ItemKey) < Count)
    {
        return false;
    }

    int32 RemovedCount = 0;

    // 뒤에서부터 돌면서 같은 ItemKey를 찾으면 삭제
    for (int32 i = ItemKeys.Num() - 1; i >= 0; --i)
    {
        if (ItemKeys[i] == ItemKey)
        {
            ItemKeys.RemoveAt(i);
            RemovedCount++;

            // 필요한 개수만큼 지웠으면 종료
            if (RemovedCount >= Count)
            {
                break;
            }
        }
    }
    OnInventoryChanged.Broadcast();

    return true;
}


bool UInventoryComponent::HasEnoughItem(FName ItemKey, int32 RequiredCount) const
{
    const int32 CurrentCount = GetItemCount(ItemKey);

    return CurrentCount >= RequiredCount;
}

const FItemData* UInventoryComponent::FindItemData(FName ItemKey) const
{
    if (!ItemDataTable || ItemKey.IsNone())
    {
        return nullptr;
    }

    return ItemDataTable->FindRow<FItemData>(
        ItemKey,
        TEXT("InventoryComponent::FindItemData")
    );
}