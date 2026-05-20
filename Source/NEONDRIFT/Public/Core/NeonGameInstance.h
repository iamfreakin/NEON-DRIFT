#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NeonTypes.h"
#include "NeonGameInstance.generated.h"

UCLASS()
class NEONDRIFT_API UNeonGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    UPROPERTY() FUpgradeState Upgrades;

    void ResetRun();
    bool TryPurchase(EUpgradeId Id, int32& InOutResources, int32 Cost, int32 MaxLevel);

    FShipStats   GetShipStats() const;
    FTurretStats GetManualTurretStats() const;
    FTurretStats GetAutoTurretStats() const;
    int32        GetAutoTurretCount() const;
};
