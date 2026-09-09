#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"


UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};


class DREAMPROTECTOR_API IInteractable
{
	GENERATED_BODY()

	
public:
	//상호작용을 요청한 Actor를 받아 상호작용 처리
	virtual void Interact(AActor* Interactor) = 0;

};