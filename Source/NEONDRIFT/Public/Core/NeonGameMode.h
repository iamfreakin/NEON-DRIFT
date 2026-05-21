#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NeonTypes.h"
#include "Sound/SoundBase.h"
#include "NeonGameMode.generated.h"

class ABase;
class UAudioComponent;
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

    // ---- Audio ----
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* WaveStartSound    = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* WaveClearSound    = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* VictorySound      = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* GameOverSound     = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* ShopBuySound      = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* ShopNavigateSound = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* ShopFailSound     = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* MonsterDeathSound = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* MonsterAttackSound= nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* ShardCollectSound = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* BlockHitSound     = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* BlockBreakSound   = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* BGMSound          = nullptr;
    UPROPERTY(EditDefaultsOnly, Category="Audio") USoundBase* RainAmbienceSound = nullptr;

    // ---- Data tables ----
    UPROPERTY(EditDefaultsOnly, Category="WaveData")   TArray<FWaveDef>    WaveTable;
    UPROPERTY(EditDefaultsOnly, Category="BlockData")  TArray<FBlockDef>   BlockTable;
    UPROPERTY(EditDefaultsOnly, Category="UpgradeData")TArray<FUpgradeDef> UpgradeTable;

    // ---- Spawn config ----
    UPROPERTY(EditDefaultsOnly, Category="BlockData") int32 TotalBlocksToSpawn  = 40;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnRadius     = 8000.f;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnMinRadius  = 2500.f;
    UPROPERTY(EditDefaultsOnly, Category="BlockData") float BlockSpawnHeight     = 1200.f;
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
    float SpawnAccum              = 0.f;
    float MonsterDeathSoundTimer  = 0.f;
    UPROPERTY() UAudioComponent* BGMAudio      = nullptr;
    UPROPERTY() UAudioComponent* AmbienceAudio = nullptr;

    void InitDefaultTables();
    void SpawnResourceField();
    void TickCombat(float Dt);
    void SpawnMonsterAtEntrance(int32 EntranceIdx);
    FVector GetSpawnLocation(ESpawnDir Dir) const;
    void RefreshAutoTurrets();
};
