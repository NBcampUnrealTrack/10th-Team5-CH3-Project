#include "ToyPlayerController.h"
//Enhanced Input의 LocalPlayer Subsystem을 사용하기위해 필요함
//BeginPlay에서 Input Mapping Context를 플레이어에게 등록할때 사용함
#include "EnhancedInputSubsystems.h"
//UEhancedInputComponent를 사용하기 위에 필요함.
//Input Action과 C++ 함수를 BindAction()으로 연결할떄 사용함.
#include "EnhancedInputComponent.h"
//카메라 시점 제한에 사용
#include "Camera/PlayerCameraManager.h"
//FInputActionValue를 사용하기 위해 필요함.
//Move 입력의 Axis2D 값을 받아올 때 사용함.
#include "InputActionValue.h"
//APlayerCharacter를 사용하기 위해 필요함.
//Controller가 현재 조종 중인 Pawn을 PlayerCharacter로 변환해서 Move()함수를 호출하기 위해 사용함
#include "PlayerCharacter.h"
//블루프린트 위젯사용 
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
//현재 레벨 이름을 가져오기 위해 필요
#include "Kismet/GameplayStatics.h"




AToyPlayerController::AToyPlayerController()
    //Input관련 포인터들을 처음에는 아무것도 가리키지 않는 상태로 초기화함.
    //이후에 BluePrint에서 실제 IMC와Input Action을 지정함
	:InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr),
	AttackAction(nullptr),
	InteraAtionAction(nullptr),
	ReloadAction(nullptr),
	InventoryAction(nullptr),
	InventoryWidget(nullptr)
{
}

// 게임이 시작되어 이 PlayerController가 활성화될 때 호출된다.
void AToyPlayerController::BeginPlay()
{   //부모클래스인 APlayerController의 BeginPlay 먼저 실행
	//부모가 기본적으로 해야하는 초기화 작업을 유지하기 위해 호출함
	Super::BeginPlay();
	//인풋 다시게임으로
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
	//마우스커서 안보이게하기
	bShowMouseCursor = false;

	//카메라 시점 제한
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = MinCameraPitch;
		PlayerCameraManager->ViewPitchMax = MaxCameraPitch;
	}

	//이 Controller와 연결된 로컬 플레이어를 가져온다. (로컬플레이어 = 게임을 직접 하는 사용자)
	//Enhanced Input의 Input MappingContext를 관리하는 Subsystem이 LocalPlay에 있음.
	//가져오기에 성공했을 경우에만 if 내부를 실행한다.
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		//LocalPlayer가 가지고있는 Enhanced Input Subsystem을 가져온다
		//이 Subsystem을 통해 Input Mapping Contect를 등록/제거 가능.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			//Blueprint에 InputMappintContext가 정상적으로 지정되있는지 확인.
			if (InputMappingContext)
			{
				//이 플레이어에 Enhanced Input 시스템에 우리가 사용할 InputMappintContext를 등록.
				//두번째 인자인 0은 MappingContext에 우선순위.
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
	//위젯 클래스가 지정되있다면
	if (HUDWidgetClass)
	{
		//실제 HUD위젯 객체 생성
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			// 생성한HUD를 화면에 표시
			HUDWidget->AddToViewport();
		}
	}

	// 실행 중인 맵 이름을 가져옵니다.
	// true: 에디터 실행 시 붙는 접두사를 제거합니다.
	const FString CurrentLevelName =
		UGameplayStatics::GetCurrentLevelName(this, true);

	// Stage1에서만 튜토리얼 위젯을 생성합니다.
	if (CurrentLevelName == TEXT("Stage1") && TutorialWidgetClass)
	{
		TutorialWidget = CreateWidget<UUserWidget>(
			this,
			TutorialWidgetClass
		);

		if (TutorialWidget)
		{
			TutorialWidget->AddToViewport(10);

			UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this);

			bShowMouseCursor = true;
		}
	}
}

//PlayerController의 입력 설정을 초기화 할때 호출되는 함수(바인딩함수)
void AToyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(

				MoveAction,
				ETriggerEvent::Triggered,
				this,
				&AToyPlayerController::Move
			);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&AToyPlayerController::Look
			);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::StartJump
			);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&AToyPlayerController::StopJump
			);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::StartSprint
			);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&AToyPlayerController::StopSprint
			);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(
				AttackAction,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::StartAttack
			);

			EnhancedInputComponent->BindAction(
				AttackAction,
				ETriggerEvent::Completed,
				this,
				&AToyPlayerController::StopAttack
			);
		}

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(
				ReloadAction,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::Reload
			);
		}
		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(
				InventoryAction,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::Inventory
			);
		}

		if (UseSlot1Action)
		{
			EnhancedInputComponent->BindAction(
				UseSlot1Action,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::UseSlot1
			);
		}

		if (UseSlot2Action)
		{
			EnhancedInputComponent->BindAction(
				UseSlot2Action,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::UseSlot2
			);
		}

		if (UseSlot3Action)
		{
			EnhancedInputComponent->BindAction(
				UseSlot3Action,
				ETriggerEvent::Started,
				this,
				&AToyPlayerController::UseSlot3
			);
		}

	}
}

//IA_Move 입력이 발생하면 호출되는 함수 플레이어 이동함수
void AToyPlayerController::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (APlayerCharacter* PlayerCharater =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharater->Move(MovementVector);
	}
}
//카메라 시점회전
void AToyPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}
//점프키를 눌럿을떄 점프
void AToyPlayerController::StartJump()
{
	if (APlayerCharacter* PlayerCharacter
		= Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->Jump();
	}
}
//점프키를뗏을때 점프 함수
void AToyPlayerController::StopJump()
{
	if (APlayerCharacter* PlayerCharacter
		= Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->StopJumping();
	}
}
//스프린트 키를 눌럿을때 함수
void AToyPlayerController::StartSprint()
{
	if (APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->StartSprint();
	}
}
//스프린트키를 뗏을때 함수
void AToyPlayerController::StopSprint()
{
	if (APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->StopSprint();
	}
}
//공격 
void AToyPlayerController::StartAttack()
{
	if (APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->StartAttack();
	}
}

void AToyPlayerController::StopAttack()
{
	if (APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->StopAttack();
	}
}

void AToyPlayerController::Reload()
{
	if (APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->ReloadMana();
	}
}

void AToyPlayerController::Inventory()
{
	if (!InventoryWidgetClass)
	{
		return;
	}

	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UUserWidget>(
			this,
			InventoryWidgetClass
		);
	}

	if (InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();

		bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		SetIgnoreMoveInput(false);
	}
	else
	{
		InventoryWidget->AddToViewport();

		bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;

		// 마우스로 UI를 클릭할 수 있으면서
		// 키보드 입력은 게임 쪽에서도 계속 받을 수 있도록 설정
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		SetInputMode(InputMode);

		// UI를 열어도 Tab 입력을 PlayerController가 계속 받을 수 있게
		SetIgnoreMoveInput(true);
	}
}

void AToyPlayerController::UseSlot1()
{
	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	// 현재 실행 중인 맵 이름 가져오기
	const FString LevelName = GetWorld()->GetMapName();

	// Stage2에서는 1번 슬롯 = 하트 태엽
	if (LevelName.Contains(TEXT("Stage2")))
	{
		PlayerCharacter->UseItem(TEXT("HeartGear"));
	}
	// 그 외(Stage1)에서는 기존 태엽 폭탄
	else
	{
		PlayerCharacter->UseItem(TEXT("WindupBomb"));
	}
}

void AToyPlayerController::UseSlot2()
{
	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	const FString LevelName = GetWorld()->GetMapName();

	if (LevelName.Contains(TEXT("Stage2")))
	{
		// Stage2 2번 슬롯 = 별사탕
		PlayerCharacter->UseItem(TEXT("StarCandy"));
	}
	else
	{
		// Stage1 2번 슬롯 = 바리케이드
		PlayerCharacter->UseItem(TEXT("Barricade"));
	}
}

void AToyPlayerController::UseSlot3()
{
	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	const FString LevelName = GetWorld()->GetMapName();

	if (LevelName.Contains(TEXT("Stage2")))
	{
		// Stage2 3번 슬롯 = 태엽 신발
		PlayerCharacter->UseItem(TEXT("GearShoes"));
	}
	else
	{
		// Stage1 3번 슬롯 = 수면등
		PlayerCharacter->UseItem(TEXT("SleepLamp"));
	}
}