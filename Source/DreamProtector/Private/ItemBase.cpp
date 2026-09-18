#include "ItemBase.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"


AItemBase::AItemBase()
{

	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(SceneRoot);
}


void AItemBase::Interact(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("Item Interact"));

	//상호작용한 Actor가 PlayerCharacter인지 확인
	APlayerCharacter* Player = Cast<APlayerCharacter>(Interactor);

	if (Player)
	{
		//Player가 가지고 있는 InventryComponent 찾기 
		UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();

		if (Inventory)
		{
			// ItemKey를 이용해서 DataTable에서 아이템 정보를 찾는다.
			const FItemData* Data = GetItemData();
			if (Data)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("DataTable Item Found: %s"),
					*Data->ItemName.ToString()
				);
			}
			else
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("Item Data Not Found: %s"),
					*ItemKey.ToString()
				);
			}

			// 기존처럼 아이템 Key를 인벤토리에 추가
			Inventory->AddItem(ItemKey);

			// 획득한 월드 아이템 제거
			Destroy();
		}
		

	}
}

const FItemData* AItemBase::GetItemData() const
{
	// DataTable이 설정되어 있지 않으면 찾을 수 없다.
	if (!ItemDataTable)
	{
		return nullptr;
	}

	// ItemKey를 Row Name으로 사용해서 데이터를 찾는다.
	return ItemDataTable->FindRow<FItemData>(
		ItemKey,
		TEXT("GetItemData")
	);
}
