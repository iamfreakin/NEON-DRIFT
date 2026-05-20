#include "NeonGameInstance.h"

void UNeonGameInstance::ResetRun()
{
    Upgrades.Levels.Empty();
}

bool UNeonGameInstance::TryPurchase(EUpgradeId Id, int32& InOutResources, int32 Cost, int32 MaxLevel)
{
    int32 Cur = Upgrades.GetLevel(Id);
    if (Cur >= MaxLevel)   return false;
    if (InOutResources < Cost) return false;
    InOutResources -= Cost;
    Upgrades.SetLevel(Id, Cur + 1);
    return true;
}

FShipStats UNeonGameInstance::GetShipStats() const
{
    FShipStats S;
    int32 Atk = Upgrades.GetLevel(EUpgradeId::Ship_Attack);
    S.AttackDamage = 1.f + Atk;
    S.AttackPower  = 1   + Atk;
    S.FireRate     = 10.f;
    S.MagnetRadius = 500.f + Upgrades.GetLevel(EUpgradeId::Ship_Magnet)  * 100.f;
    S.MaxSpeed     = 4000.f + Upgrades.GetLevel(EUpgradeId::Ship_Speed)  * 1000.f;
    S.MaxHP        = 3.f   + Upgrades.GetLevel(EUpgradeId::Ship_HP)     * 2.f;
    S.Acceleration = 12000.f;
    S.LinearDrag   = 2.f;
    return S;
}

FTurretStats UNeonGameInstance::GetManualTurretStats() const
{
    FTurretStats T;
    T.AttackDamage  = 1.f + Upgrades.GetLevel(EUpgradeId::Turret_Attack);
    T.FireRate      = 5.f + Upgrades.GetLevel(EUpgradeId::Turret_FireRate);
    T.RotationSpeed = 30.f + Upgrades.GetLevel(EUpgradeId::Turret_Rotate) * 90.f;
    T.Range         = 6000.f;
    return T;
}

FTurretStats UNeonGameInstance::GetAutoTurretStats() const
{
    FTurretStats T;
    T.AttackDamage  = 0.5f + Upgrades.GetLevel(EUpgradeId::Turret_Attack) * 0.5f;
    T.FireRate      = 3.f  + Upgrades.GetLevel(EUpgradeId::Turret_FireRate) * 0.5f;
    T.RotationSpeed = 120.f;
    T.Range         = 5000.f;
    return T;
}

int32 UNeonGameInstance::GetAutoTurretCount() const
{
    return 1 + Upgrades.GetLevel(EUpgradeId::AutoTurret_Count);
}
