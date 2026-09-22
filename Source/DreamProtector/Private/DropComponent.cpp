#include "DropComponent.h"

UDropComponent::UDropComponent()
{
	//필요 없음
	PrimaryComponentTick.bCanEverTick = false;
}


void UDropComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UDropComponent::DropItem()
{
	//드롭 가능한 아이템이 있는지 확인
	if (DropItemClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DropComponent: DropItemClasses is Empty."));
		return;
	}

	//0.0 ~ 1.0 사이 랜덤 실수 생성
	const float RandomValue = FMath::FRand();

	//랜덤값이 설정된 드롭 확률보다 크면 드롭 X
	if (RandomValue > DropChance)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropComponent: Drop Fail, Random: %.2f/ Chance: %.2f"), RandomValue, DropChance);
		return;
	}

	//0 ~ 마지막 배열 Index 중 하나를 랜덤으로 선택
	const int32 RandomIndex = FMath::RandRange(0, DropItemClasses.Num() - 1);

	//선택된 아이템 클래스 가져오기
	TSubclassOf<AActor>SelectedItemClass =
		DropItemClasses[RandomIndex];

	//배열에 None이 들어가 있는경우 일단 검사
	if (!SelectedItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropComponent: Selected Item class is null."));
		return;
	}

	//DropComponent가 Monster에 붙어있다면 해당 Monster Actor를 반환
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropComponent: Owner is null."));
		return;
	}

	//기본적으로 몬스터가 죽은 위치를 사용
	FVector DropLocation = OwnerActor->GetActorLocation();

	//아이템이 바닥 안쪽에 생성되는것을 방지 하기 위해 살짝 위에 생성
	DropLocation.Z += 30.0f;

	const FRotator DropRotation = FRotator::ZeroRotator;

	//아이템 스폰 
	AActor* SpawnedItem = GetWorld()->SpawnActor<AActor>(
		SelectedItemClass,
		DropLocation,
		DropRotation
	);

	//스폰 성공 여부 확인용 테스트
	if (SpawnedItem)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("DropComponent: Item dropped -> %s"),
			*SpawnedItem->GetName()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("DropComponent: Failed to spawn item.")
		);
	}
}

