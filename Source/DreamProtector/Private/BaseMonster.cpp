#include "BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"


ABaseMonster::ABaseMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 100.0f;
	
	AttackDamage = 10.0f;
	MoveSpeed = 300.0f;
}

void ABaseMonster::BeginPlay()   
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;                                       // 현제 체력 최대 체력으로 초기화

	if (GetCharacterMovement()) 
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;    // 이동속도 스탯 적용
	}
}

void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 사거리 체크 후 Attack() 로직 (원거리)
	// 자식 클래스마다 방식을 (오버랩 / 사거리) 각 구현
}

void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)    // 외부에서 타겟 지정할 때 호출
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);                            // 몬스터는 플레이어 입력을 받지 않아 필요없는 부분
}

void ABaseMonster::SetTarget(AActor* InTarget)
{
	Target = InTarget;
}

void ABaseMonster::TakeDamage(float DamageAmount)         // 데미지 받을 때 외부(공격한 쪽)에서 호출
{
	if (CurrentHealth <= 0.0f) return;                    // 이미 죽었으면 무시 retrun (중복 데미지 막기)

	CurrentHealth -= DamageAmount;                        // 받은 데미지만큼 체력 감소

	if (CurrentHealth <= 0.0f)
	{
		Die();                                            // 체력 0이면 사망
	}
}

void ABaseMonster::Attack_Implementation()
{

}

void ABaseMonster::Die()                                  // 체력 0이하가 되면 takeDamage()에서 호출
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();        // 사망 후 더 이상 움직이지 않도록 멈춤
	}
	SetActorEnableCollision(false);                       // 충돌 판정 끄기

	// 애니메이션이난 이펙트 재생 및 웨이브에 알리는 곳

	Destroy();                                            // 액터 제거
}

