#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "ItemData.h"
#include "ItemBase.generated.h"


UCLASS()
class DREAMPROTECTOR_API AItemBase : public AActor, public IInteractable
{
  GENERATED_BODY()

public:
  AItemBase();

  virtual void Interact(AActor* Interactor) override;
  // ItemKey를 이용해 DataTable에서 아이템 정보를 찾는다.
  const FItemData* GetItemData() const;
protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent* SceneRoot;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent* StaticMeshComp;

  // 어떤 아이템인지 구분하는 Key
  // DT_ItemData의 Row Name과 같은 값을 사용한다.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  FName ItemKey;

  // 아이템 정보를 가지고 있는 DataTable
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  TObjectPtr<UDataTable> ItemDataTable;
};
