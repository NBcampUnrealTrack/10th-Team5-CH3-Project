#include "Suicide.h"

ASuicide::ASuicide()
{
	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));

	ExplosionCollision->SetupAttachment(RootComponent);

	ExplosionCollision->SetSphereRadius(150.0f);

	ExplosionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ExplosionCollision->SetGenerateOverlapEvents(true);
}

void ASuicide::Attack_Implementation()
{
	// 자폭병 공격 로직
}