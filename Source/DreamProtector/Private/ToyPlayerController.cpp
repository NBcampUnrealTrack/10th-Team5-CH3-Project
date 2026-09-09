#include "ToyPlayerController.h"
//Enhanced Input의 LocalPlayer Subsystem을 사용하기위해 필요함
//BeginPlay에서 Input Mapping Context를 플레이어에게 등록할때 사용함
#include "EnhancedInputSubsystems.h"
//UEhancedInputComponent를 사용하기 위에 필요함.
//Input Action과 C++ 함수를 BindAction()으로 연결할떄 사용함.
#include "EnhancedInputComponent.h"
//FInputActionValue를 사용하기 위해 필요함.
//Move 입력의 Axis2D 값을 받아올 때 사용함.
#include "InputActionValue.h"
//APlayerCharacter를 사용하기 위해 필요함.
//Controller가 현재 조종 중인 Pawn을 PlayerCharacter로 변환해서 Move()함수를 호출하기 위해 사용함
#include "PlayerCharacter.h"

AToyPlayerController::AToyPlayerController()
    //Input관련 포인터들을 처음에는 아무것도 가리키지 않는 상태로 초기화함.
    //이후에 BluePrint에서 실제 IMC와Input Action을 지정함
	:InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr)
{

}

// 게임이 시작되어 이 PlayerController가 활성화될 때 호출된다.
void AToyPlayerController::BeginPlay()
{   //부모클래스인 APlayerController의 BeginPlay 먼저 실행
	//부모가 기본적으로 해야하는 초기화 작업을 유지하기 위해 호출함
	Super::BeginPlay();

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

