#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "CraftingComponent.generated.h"


class UInventoryComponent;
// 제작에 필요한 재료 하나
USTRUCT(BlueprintType)
struct FCraftingIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};

// 제작 레시피 하나
USTRUCT(BlueprintType)
struct FCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()

	// 필요한 모든 재료
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCraftingIngredient> Ingredients;

	// 제작 결과 아이템
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ResultItemKey;

	// 제작 결과 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ResultCount = 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DREAMPROTECTOR_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCraftingComponent();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool CraftItem(const FCraftingRecipe& Recipe);
	

protected:
	virtual void BeginPlay() override;

public:	
	
	

		
};
