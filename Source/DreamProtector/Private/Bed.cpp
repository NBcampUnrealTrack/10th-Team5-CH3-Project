#include "Bed.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ABed::ABed()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	BedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedMesh"));
	RootComponent = BedMesh;

	BedCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BedCollision"));
	BedCollision->SetupAttachment(RootComponent);
	BedCollision->SetBoxExtent(FVector(100.0f, 150.0f, 70.0f));
}


void ABed::BeginPlay()
{

	Super::BeginPlay();
	CurrentStress = 0;
	IncreaseStress(30);
}
//스트레스 증가
void ABed::IncreaseStress(int32 Amount)
{
	CurrentStress += Amount;
	//현재스트레스가 최대스트레스보다 커지지않도록 제한
	CurrentStress = FMath::Clamp(CurrentStress, 0, MaxStress);

	OnStressChganged.Broadcast(CurrentStress, MaxStress);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Bed Stress : %d / %d"),
		CurrentStress,
		MaxStress
	);
}

int32 ABed::GetCurrentStress() const
{
	return CurrentStress;
}

int32 ABed::GetMaxStress() const
{
	return MaxStress;
}