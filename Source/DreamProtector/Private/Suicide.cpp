#include "Suicide.h"

ASuicide::ASuicide()
{
	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));

	ExplosionCollision->SetupAttachment(RootComponent);

	ExplosionCollision->SetSphereRadius(150.0f);
	//이 컴포넌트는 충돌로 물리 반응은 하지 않고, 충돌/겹침 여부를 검색(Query)하는 용도로만 사용하겠다
	ExplosionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ExplosionCollision->SetGenerateOverlapEvents(true);
}

void ASuicide::Attack_Implementation()
{
	// 자폭병 공격 로직
}