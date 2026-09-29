#include "SpawnVolume.h"
#include "Components/BoxComponent.h"

ASpawnVolume::ASpawnVolume()
{
	//스폰 영역 박스 컴포 생성
	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));

	//스폰볼륨 액터의 루트컴포를 스폰박스로 지정
	RootComponent = SpawnBox;
}

FVector ASpawnVolume::GetRandomSpawnLocation() const
{
    // SpawnBox의 월드 위치를 가져온다.
    const FVector Origin = SpawnBox->GetComponentLocation();

    // SpawnBox의 실제 월드 크기(절반)를 가져온다.
    const FVector Extent = SpawnBox->GetScaledBoxExtent();

    // Box 범위 안에서 X, Y, Z가 각각 랜덤인 위치를 반환한다.
    return Origin + FVector(
        FMath::FRandRange(-Extent.X, Extent.X),
        FMath::FRandRange(-Extent.Y, Extent.Y),
        FMath::FRandRange(-Extent.Z, Extent.Z)
    );
}