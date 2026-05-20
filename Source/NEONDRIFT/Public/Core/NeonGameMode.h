#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NeonTypes.h"
#include "NeonGameMode.generated.h"

class ABase;
class ASpawnPoint;
class AAutoTurret;
class AManualTurret;
class AResourceBlock;
class AMonster;
class APlayerShip;

UCLASS()
class NEONDRIFT_API ANeonGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ANeonGameMode();

    // ---- Game state ----
    EGamePhase Phase                = EGamePhase::PreWave;
    int32      WaveIndex            = 0;
    int32      Resources            = 0;
    float      GatherTimer          = 60.f;
    int32      AliveMonsters        = 0;
    int32      SpawnedThisWave      = 0;
    int32      TotalMonstersThisWave= 0;

    // ---- Data tables ----
    UPROPERTY(EditDefaultsOnly, Category="WaveData")   TArray<FWaveDef>    WaveTable;
    UPROPERTY(EditDefaultsOnly, Category="BlockData")  TArray<FBlockDef>   BlockTable;
    UPROPERTY(EditDefaultsOnly, Category="UpgradeData")TArray<FUpgradeDef> UpgradeTable;

    // ---- Spawn config ----
    UPROPERTY(EditDefaultsOnly, Category="BlockData") int32 TotalBlocksToSpawn  = 40;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnRadius     = 5000.f;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnMinRadius  = 1200.f;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnHeight     = 700.f;
    UPROPERTY(EditDefaultsOnly, Category="WaveData")  float SpawnDistance        = 8000.f;
    UPROPERTY(EditDefaultsOnly, Category="WaveData")  float MonsterSpawnHeight   = 500.f;

    // ---- Placed actor refs (gathered via TActorIterator in BeginPlay) ----
    UPROPERTY() ABase*               Base         = nullptr;
    UPROPERTY() TArray<ASpawnPoint*> SpawnPoints;
    UPROPERTY() TArray<AAutoTurret*> AutoTurrets;
    UPROPERTY() AManualTurret*       ManualTurret = nullptr;

    // ---- GameMode interface ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void EnterPhase(EGamePhase NewPhase);
    void RequestStartWave();
    void AddResources(int32 Amount);
    void OnMonsterKilled();
    void OnBaseDestroyed();
    void OnWaveCleared();
    void ApplyUpgrade(EUpgradeId Id);
    void ContinueToNextWave();
    FUpgradeDef* FindUpgradeDef(EUpgradeId Id);

private:
    float SpawnAccum = 0.f;

    void InitDefaultTables();
    void SpawnResourceField();
    void TickCombat(float Dt);
    void SpawnMonsterAtEntrance(int32 EntranceIdx);
    FVector GetSpawnLocation(ESpawnDir Dir) const;
    void RefreshAutoTurrets();
};
