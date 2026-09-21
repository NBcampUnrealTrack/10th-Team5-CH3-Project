#include "BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WaveManager.h"
#include "WaveManager2.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAIController.h"

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

void ABaseMonster::SetIsPhase2Monster(bool bInIsPhase2Monster)
{
	bIsPhase2Monster = bInIsPhase2Monster;
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
	{
		GetCharacterMovement()->DisableMovement();
	}

	SetActorEnableCollision(false);

	// 2-2 몬스터인지 확인
	if (IsPhase2Monster())
	{
		if (AWaveManager2* WaveManager2 =
			Cast<AWaveManager2>(
				UGameplayStatics::GetActorOfClass(
					GetWorld(),
					AWaveManager2::StaticClass()
				)
			))
		{
			WaveManager2->OnPhase2MonsterKilled();
		}
	}
	else
	{
		// 기존 Stage 1 몬스터
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
	}

	Destroy();
}

