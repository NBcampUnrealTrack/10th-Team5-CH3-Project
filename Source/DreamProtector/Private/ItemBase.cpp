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
			//이 아이템이 가지고 있는 ItemKey를 인벤토리에 추가
			Inventory->AddItem(ItemKey);

			//인벤토리에 추가된 뒤 월드에서 아이템 제거
			Destroy();
		}
		

	}
}


