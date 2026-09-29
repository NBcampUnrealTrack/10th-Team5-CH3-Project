#include "ItemBase.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"


AItemBase::AItemBase()
{

	PrimaryActorTick.bCanEverTick = false;

	// 아이템의 루트 컴포넌트
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 실제 아이템 외형을 보여주는 Static Mesh
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(SceneRoot);

	// 아이템 Mesh의 Collision을 완전히 비활성화한다 플레이어, 몬스터, 발사체 등을 물리적으로 막지 않음
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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
