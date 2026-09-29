#include "CraftingComponent.h"
#include "InventoryComponent.h"

UCraftingComponent::UCraftingComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

}



void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	
	
}



bool UCraftingComponent::CraftItem(const FCraftingRecipe& Recipe)
{
    // CraftingComponent를 가지고 있는 Actor 찾기
    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return false;
    }

    // Owner의 InventoryComponent 찾기
    UInventoryComponent* Inventory =
        Owner->FindComponentByClass<UInventoryComponent>();

    if (!Inventory)
    {
        return false;
    }

    // 1. 모든 재료가 충분한지 먼저 확인
    for (const FCraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        if (!Inventory->HasEnoughItem(
            Ingredient.ItemKey,
            Ingredient.Count))
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Craft Failed - Not Enough: %s"),
                *Ingredient.ItemKey.ToString()
            );

            return false;
        }
    }

    // 2. 모든 재료가 있다는 것을 확인했으므로 재료 소비
    for (const FCraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        Inventory->RemoveItems(
            Ingredient.ItemKey,
            Ingredient.Count
        );
    }

    // 3. 제작 결과 아이템 지급
    for (int32 i = 0; i < Recipe.ResultCount; ++i)
    {
        Inventory->AddItem(Recipe.ResultItemKey);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Craft Success: %s x%d"),
        *Recipe.ResultItemKey.ToString(),
        Recipe.ResultCount
    );

    return true;
}