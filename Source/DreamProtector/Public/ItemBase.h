#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"


UCLASS()
class DREAMPROTECTOR_API AItemBase : public AActor, public IInteractable
{
  GENERATED_BODY()

public:
  AItemBase();

  virtual void Interact(AActor* Interactor) override;

protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent* SceneRoot;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent* StaticMeshComp;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  FName ItemKey;
};
