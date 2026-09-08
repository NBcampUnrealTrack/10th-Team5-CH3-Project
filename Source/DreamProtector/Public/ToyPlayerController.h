#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ToyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
UCLASS()
class DREAMPROTECTOR_API AToyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AToyPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;

private:
	void BeginPlay();
};
