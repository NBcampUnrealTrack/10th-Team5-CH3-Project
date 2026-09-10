 #include "ItemBase.h"


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

	Destroy();
}


