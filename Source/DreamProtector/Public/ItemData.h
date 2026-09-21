#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"


UENUM(BlueprintType)
enum class EItemUseType : uint8
{
    None        UMETA(DisplayName = "사용 불가"),
    WindupBomb  UMETA(DisplayName = "태엽 폭탄"),
    Barricade   UMETA(DisplayName = "바리케이드"),
    SleepLamp   UMETA(DisplayName = "수면등"),

    HeartGear   UMETA(DisplayName = "하트 태엽"),
    StarCandy   UMETA(DisplayName = "별사탕"),
    GearShoes   UMETA(DisplayName = "태엽 신발")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Material    UMETA(DisplayName = "재료"),
    Consumable  UMETA(DisplayName = "소비 아이템")
};

USTRUCT(BlueprintType)
struct DREAMPROTECTOR_API FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // 아이템을 구분하기 위한 고유 Key
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemKey;

    // 플레이어 화면에 표시할 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText ItemName;

    // 플레이어 화면에 표시할 아이템 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Description;
    //아이템 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType ItemType = EItemType::Material;
    //아이템 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TObjectPtr<UTexture2D>Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemUseType UseType = EItemUseType::None;
};