#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NeonTypes.generated.h"

// ---- Enums ----

UENUM(BlueprintType)
enum class EBlockType : uint8 { Gold, Iron, Diamond };

UENUM(BlueprintType)
enum class EMonsterColor : uint8 { Orange, Cyan };

UENUM(BlueprintType)
enum class EMonsterVariant : uint8 { Ground, Flying };

UENUM(BlueprintType)
enum class EGamePhase : uint8 { PreWave, Gather, Combat, Shop, GameOver, Victory };

UENUM(BlueprintType)
enum class ESpawnDir : uint8 { North, East, South, West };

UENUM(BlueprintType)
enum class EUpgradeId : uint8
{
    Ship_Magnet      UMETA(DisplayName="Magnet Range"),
    Ship_Attack      UMETA(DisplayName="Ship Attack"),
    Ship_Speed       UMETA(DisplayName="Ship Speed"),
    Ship_HP          UMETA(DisplayName="Ship HP"),
    Turret_Rotate    UMETA(DisplayName="Turret RotSpeed"),
    Turret_Attack    UMETA(DisplayName="Turret Attack"),
    Turret_FireRate  UMETA(DisplayName="Turret FireRate"),
    AutoTurret_Count UMETA(DisplayName="Auto Turret +1"),
};

// ---- Stat Structs ----

USTRUCT()
struct FShipStats
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) float AttackDamage  = 1.f;
    UPROPERTY(EditAnywhere) int32 AttackPower   = 1;
    UPROPERTY(EditAnywhere) float FireRate      = 10.f;    // shots/sec
    UPROPERTY(EditAnywhere) float MagnetRadius  = 200.f;   // uu (2m)
    UPROPERTY(EditAnywhere) float MaxSpeed      = 2000.f;  // uu/s
    UPROPERTY(EditAnywhere) float MaxHP         = 3.f;
    UPROPERTY(EditAnywhere) float Acceleration  = 5000.f;
    UPROPERTY(EditAnywhere) float LinearDrag    = 2.f;     // 1/sec drag coefficient
};

USTRUCT()
struct FTurretStats
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) float AttackDamage  = 1.f;
    UPROPERTY(EditAnywhere) float FireRate      = 5.f;
    UPROPERTY(EditAnywhere) float RotationSpeed = 30.f;   // deg/sec (manual turret cap)
    UPROPERTY(EditAnywhere) float Range         = 6000.f; // uu
};

// ---- Upgrade State ----

USTRUCT()
struct FUpgradeState
{
    GENERATED_BODY()
    UPROPERTY() TMap<EUpgradeId, int32> Levels;

    int32 GetLevel(EUpgradeId Id) const
    {
        const int32* L = Levels.Find(Id);
        return L ? *L : 0;
    }
    void SetLevel(EUpgradeId Id, int32 Lvl) { Levels.Add(Id, Lvl); }
};

// ---- Inline Data Tables ----

USTRUCT()
struct FWaveDef
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) TArray<ESpawnDir> Entrances;
    UPROPERTY(EditAnywhere) int32  TotalMonsters  = 5;
    UPROPERTY(EditAnywhere) float  MonsterHP      = 1.f;
    UPROPERTY(EditAnywhere) float  SpawnInterval  = 2.f;   // sec
    UPROPERTY(EditAnywhere) float  MoveSpeed      = 1000.f; // uu/s
    UPROPERTY(EditAnywhere) float  AttackDPS      = 1.f;
};

USTRUCT()
struct FBlockDef
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) EBlockType    Type         = EBlockType::Gold;
    UPROPERTY(EditAnywhere) FLinearColor  EmissiveColor= FLinearColor(1.f, 0.5f, 0.f);
    UPROPERTY(EditAnywhere) int32         RequiredPower= 1;
    UPROPERTY(EditAnywhere) float         HP           = 3.f;
    UPROPERTY(EditAnywhere) int32         ShardMin     = 5;
    UPROPERTY(EditAnywhere) int32         ShardMax     = 8;
    UPROPERTY(EditAnywhere) int32         ShardValue   = 1;
    UPROPERTY(EditAnywhere) float         SpawnWeight  = 1.f;
};

USTRUCT()
struct FUpgradeDef
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) EUpgradeId  Id          = EUpgradeId::Ship_Magnet;
    UPROPERTY(EditAnywhere) FString     DisplayName = TEXT("Upgrade");
    UPROPERTY(EditAnywhere) FString     EffectDesc  = TEXT("+X per level");
    UPROPERTY(EditAnywhere) int32       Cost        = 15;
    UPROPERTY(EditAnywhere) int32       MaxLevel    = 4;
};

// ---- IDamageable Interface ----

UINTERFACE(MinimalAPI)
class UDamageable : public UInterface { GENERATED_BODY() };

class NEONDRIFT_API IDamageable
{
    GENERATED_BODY()
public:
    virtual void TakeHit(float Damage, int32 AttackerPower) = 0;
};
