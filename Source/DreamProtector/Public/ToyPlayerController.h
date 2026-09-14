#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "ToyPlayerController.generated.h"


class UInputMappingContext;
class UInputAction;
class UUserWidget;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* InteraAtionAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY()
	UUserWidget* InventoryWidget;
  
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY()
	UUserWidget* HUDWidget;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void StartJump();
	UFUNCTION()
	void StopJump();
	UFUNCTION()
	void StartSprint();
	UFUNCTION()
	void StopSprint();
	UFUNCTION()
	void StartAttack();
	UFUNCTION()
	void StopAttack();
	UFUNCTION()
	void Reload();

	UFUNCTION()
	void Inventory();
};
