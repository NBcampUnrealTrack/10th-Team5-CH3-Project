#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "StaffBase.generated.h"



UCLASS()
class DREAMPROTECTOR_API AStaffBase : public AWeaponBase
{
	GENERATED_BODY()
	


protected:

	virtual void Attack() override;





};
