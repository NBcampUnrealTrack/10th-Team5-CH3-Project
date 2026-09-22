#include "BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAIController.h"
#include "DropComponent.h"

ABaseMonster::ABaseMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 100.0f;
	
	AttackDamage = 10.0f;
	AttackInterval = 1.5f;
	MoveSpeed = 300.0f;

	// 해당 몬스터가 스폰될 때 방의할 AI 컨트롤러 지정
	AIControllerClass = AMonsterAIController::StaticClass();
	// 레벨에 미리 배치 OR 스폰하면 자동으로 AI가 빙의 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	//몬스터 사망시 아이템 드롭을 담당하는 컴포넌트 생성
	DropComponent = CreateDefaultSubobject<UDropComponent>(TEXT("DropComponent"));
}

void ABaseMonster::BeginPlay()   
{
	Super::BeginPlay();
	// 현제 체력 최대 체력으로 초기화
	CurrentHealth = MaxHealth;                                       

	if (GetCharacterMovement()) 
	{
		// 이동속도 스탯 적용
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;    
	}
}

void ABaseMonster::SetMoveSpeed(float NewSpeed)
{
	MoveSpeed = NewSpeed;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
}

void ABaseMonster::Tick(float DeltaTime)
{
	// 사거리 체크 후 Attack() 로직 (원거리)
	// 자식 클래스마다 방식을 (오버랩 / 사거리) 각 구현
	Super::Tick(DeltaTime);

}
// 외부에서 타겟 지정할 때 호출
void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)    
{// 몬스터는 플레이어 입력을 받지 않아 필요없는 부분
	Super::SetupPlayerInputComponent(PlayerInputComponent);                            
}

void ABaseMonster::SetTarget(AActor* InTarget)
{
	Target = InTarget;
}

void ABaseMonster::Attack_Implementation()
{
	UE_LOG(LogTemp, Error, TEXT("[BaseMonster] 부모의 빈 Attack_Implementation 호출됨! (이게 찍히면 AMelee가 오버라이드 못 하고 있는 것)"));
	// 기본 공격 로직
}
// 데미지 받을 때 외부(공격한 쪽)에서 호출
void ABaseMonster::TakeDamage(float DamageAmount)         
{// 이미 죽었으면 무시 retrun (중복 데미지 막기)
	if (CurrentHealth <= 0.0f) return;                        
	// 받은 데미지만큼 체력 감소
	CurrentHealth -= DamageAmount;           

	// 데미지 받을 때마다 블루프린트 호출~
	OnHit();

	if (CurrentHealth <= 0.0f)
	{ // 체력 0이면 사망
		Die();
	}
}
// 체력 0이하가 되면 takeDamage()에서 호출
void ABaseMonster::Die_Implementation()                                 
{
	UE_LOG(LogTemp, Warning, TEXT("Die_Implementation Called"));
	if (GetCharacterMovement())
	{// 사망 후 더 이상 움직이지 않도록 멈춤
		GetCharacterMovement()->DisableMovement();        
	}// 충돌 판정 끄기
	// 애니메이션이난 이펙트 재생 및 웨이브에 알리는 곳
	SetActorEnableCollision(false);                       

	
	//아이템 드롭 컴포넌트가 있을시 죽은위치 기준으로 아이템 드롭
	if (DropComponent)
	{
		DropComponent->DropItem();
	}


	// WaveManager에게 몬스터가 죽었다고 알림
	if (AWaveManager* WaveManager =
		Cast<AWaveManager>(
			UGameplayStatics::GetActorOfClass(
				GetWorld(),
				AWaveManager::StaticClass()
			)
		))
	{
		WaveManager->OnMonsterKilled();
	}

	else
	{
		UE_LOG(LogTemp, Error, TEXT("WaveManager NOT Found"));
	}
	// 액터 제거
	Destroy();                                            
}

