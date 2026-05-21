#include "NeonGameMode.h"
#include "Components/AudioComponent.h"
#include "NeonGameInstance.h"
#include "NeonBase.h"
#include "SpawnPoint.h"
#include "AutoTurret.h"
#include "ManualTurret.h"
#include "ResourceBlock.h"
#include "Monster.h"
#include "PlayerShip.h"
#include "NeonHUD.h"
#include "NeonPlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ANeonGameMode::ANeonGameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultPawnClass       = APlayerShip::StaticClass();
    HUDClass               = ANeonHUD::StaticClass();
    PlayerControllerClass  = ANeonPlayerController::StaticClass();
}

void ANeonGameMode::BeginPlay()
{
    Super::BeginPlay();

    InitDefaultTables();

    for (TActorIterator<ABase>         It(GetWorld()); It; ++It) { Base = *It; break; }
    for (TActorIterator<ASpawnPoint>   It(GetWorld()); It; ++It) SpawnPoints.Add(*It);
    for (TActorIterator<AAutoTurret>   It(GetWorld()); It; ++It) AutoTurrets.Add(*It);
    for (TActorIterator<AManualTurret> It(GetWorld()); It; ++It) { ManualTurret = *It; break; }

    if (Base) Base->GameModeRef = this;

    RefreshAutoTurrets();
    SpawnResourceField();
    EnterPhase(EGamePhase::PreWave);

    if (BGMSound)          BGMAudio      = UGameplayStatics::SpawnSound2D(this, BGMSound);
    if (RainAmbienceSound) AmbienceAudio = UGameplayStatics::SpawnSound2D(this, RainAmbienceSound);
}

void ANeonGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (MonsterDeathSoundTimer > 0.f) MonsterDeathSoundTimer -= DeltaSeconds;

    switch (Phase)
    {
    case EGamePhase::Gather:
        GatherTimer -= DeltaSeconds;
        if (GatherTimer <= 0.f) EnterPhase(EGamePhase::Combat);
        break;

    case EGamePhase::Combat:
        TickCombat(DeltaSeconds);
        if (TotalMonstersThisWave > 0
            && SpawnedThisWave >= TotalMonstersThisWave
            && AliveMonsters <= 0)
        {
            OnWaveCleared();
        }
        break;

    default: break;
    }
}

void ANeonGameMode::EnterPhase(EGamePhase NewPhase)
{
    Phase = NewPhase;

    switch (NewPhase)
    {
    case EGamePhase::PreWave:
        break;

    case EGamePhase::Gather:
        GatherTimer = 40.f;
        // Destroy leftover blocks, respawn field
        for (TActorIterator<AResourceBlock> It(GetWorld()); It; ++It) It->Destroy();
        SpawnResourceField();
        break;

    case EGamePhase::Combat:
    {
        const FWaveDef& W   = WaveTable[WaveIndex];
        TotalMonstersThisWave = W.TotalMonsters;
        AliveMonsters        = 0;
        SpawnedThisWave      = 0;
        SpawnAccum           = 0.f;
        if (WaveStartSound) UGameplayStatics::PlaySound2D(this, WaveStartSound);
        break;
    }
    case EGamePhase::Shop:
        if (WaveClearSound) UGameplayStatics::PlaySound2D(this, WaveClearSound);
        if (ManualTurret) ManualTurret->SetPlayerBoarded(false);
        if (ANeonPlayerController* PC = Cast<ANeonPlayerController>(
                UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (PC->BoardedTurret)
            {
                PC->BoardedTurret = nullptr;
                PC->SetViewTargetWithBlend(PC->GetPawn(), 0.25f);
            }
            UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
            PC->ShopCursorIndex = 0;
            for (int32 i = 0; i < UpgradeTable.Num(); i++)
            {
                if (!GI || GI->Upgrades.GetLevel(UpgradeTable[i].Id) < UpgradeTable[i].MaxLevel)
                { PC->ShopCursorIndex = i; break; }
            }
        }
        break;

    case EGamePhase::GameOver:
    case EGamePhase::Victory:
        if (ManualTurret) ManualTurret->SetPlayerBoarded(false);
        if (BGMAudio)      BGMAudio->FadeOut(2.0f, 0.0f);
        if (AmbienceAudio) AmbienceAudio->FadeOut(2.0f, 0.0f);
        break;

    default: break;
    }
}

void ANeonGameMode::RequestStartWave()
{
    if (Phase == EGamePhase::PreWave)      EnterPhase(EGamePhase::Gather);
    else if (Phase == EGamePhase::Gather)  EnterPhase(EGamePhase::Combat);
}

void ANeonGameMode::AddResources(int32 Amount)
{
    Resources += Amount;
    if (ShardCollectSound) UGameplayStatics::PlaySound2D(this, ShardCollectSound);
}

void ANeonGameMode::OnMonsterKilled()
{
    AliveMonsters = FMath::Max(0, AliveMonsters - 1);
    if (MonsterDeathSound && MonsterDeathSoundTimer <= 0.f)
    {
        UGameplayStatics::PlaySound2D(this, MonsterDeathSound);
        MonsterDeathSoundTimer = 0.15f;
    }
}

void ANeonGameMode::OnBaseDestroyed()
{
    if (GameOverSound) UGameplayStatics::PlaySound2D(this, GameOverSound);
    EnterPhase(EGamePhase::GameOver);
}

void ANeonGameMode::OnWaveCleared()
{
    if (WaveIndex >= WaveTable.Num() - 1)
    {
        if (VictorySound) UGameplayStatics::PlaySound2D(this, VictorySound);
        EnterPhase(EGamePhase::Victory);
    }
    else
        EnterPhase(EGamePhase::Shop);
}

void ANeonGameMode::ApplyUpgrade(EUpgradeId Id)
{
    FUpgradeDef* Def = FindUpgradeDef(Id);
    if (!Def) return;

    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    if (!GI) return;

    if (!GI->TryPurchase(Id, Resources, Def->Cost, Def->MaxLevel))
    {
        if (ShopFailSound) UGameplayStatics::PlaySound2D(this, ShopFailSound);
        return;
    }
    if (ShopBuySound) UGameplayStatics::PlaySound2D(this, ShopBuySound);

    // Apply to live actors
    if (APlayerShip* Ship = Cast<APlayerShip>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
        Ship->ApplyStats(GI->GetShipStats());

    if (ManualTurret) ManualTurret->ApplyStats(GI->GetManualTurretStats());

    RefreshAutoTurrets();
    FTurretStats AutoStats = GI->GetAutoTurretStats();
    for (AAutoTurret* AT : AutoTurrets)
        if (AT && AT->bEnabled) AT->ApplyStats(AutoStats);
}

void ANeonGameMode::ContinueToNextWave()
{
    if (Phase != EGamePhase::Shop) return;
    WaveIndex++;
    EnterPhase(EGamePhase::PreWave);
}

FUpgradeDef* ANeonGameMode::FindUpgradeDef(EUpgradeId Id)
{
    for (FUpgradeDef& D : UpgradeTable)
        if (D.Id == Id) return &D;
    return nullptr;
}

// ---- Private helpers ----

void ANeonGameMode::TickCombat(float Dt)
{
    if (SpawnedThisWave >= TotalMonstersThisWave || !WaveTable.IsValidIndex(WaveIndex)) return;

    const FWaveDef& W = WaveTable[WaveIndex];
    if (W.Entrances.Num() == 0) return;

    SpawnAccum += Dt;
    if (SpawnAccum >= W.SpawnInterval)
    {
        SpawnAccum -= W.SpawnInterval;
        int32 EntrIdx = SpawnedThisWave % W.Entrances.Num();
        SpawnMonsterAtEntrance(EntrIdx);
        SpawnedThisWave++;
    }
}

void ANeonGameMode::SpawnMonsterAtEntrance(int32 EntranceIdx)
{
    const FWaveDef& W = WaveTable[WaveIndex];
    ESpawnDir Dir     = W.Entrances[EntranceIdx];
    FVector   Loc     = GetSpawnLocation(Dir);
    // Override Z so monsters always spawn above ground regardless of SpawnPoint placement
    Loc.Z = (Base ? Base->GetActorLocation().Z : 0.f) + MonsterSpawnHeight;

    AMonster* M = GetWorld()->SpawnActor<AMonster>(AMonster::StaticClass(), Loc, FRotator::ZeroRotator);
    if (!M) return;

    M->HP        = W.MonsterHP;
    M->MaxHP     = W.MonsterHP;
    M->MoveSpeed = W.MoveSpeed;
    M->AttackDPS = W.AttackDPS;
    M->Color     = FMath::RandBool() ? EMonsterColor::Orange : EMonsterColor::Cyan;
    M->TargetLoc = Base ? Base->GetActorLocation() : FVector::ZeroVector;
    M->BaseRef   = Base;
    M->GameModeRef = this;

    // A: 진입각 오프셋 — 스폰→기지 방향의 수직벡터로 랜덤 편향
    if (Base)
    {
        FVector ToBase = (Base->GetActorLocation() - Loc).GetSafeNormal();
        FVector Perp   = FVector::CrossProduct(ToBase, FVector::UpVector).GetSafeNormal();
        M->TargetOffset = Perp * FMath::FRandRange(-1500.f, 1500.f);
    }
    // B: 사인파 파라미터 랜덤화
    M->WanderPhase = FMath::FRandRange(0.f, 2.f * PI);
    M->WanderFreq  = FMath::FRandRange(0.6f, 1.4f);

    // 개체별 속도 산포 + 흔들림 강도 + 크기 + 돌진 대기 시간 스태거
    M->MoveSpeed       *= FMath::FRandRange(0.7f, 1.3f);
    M->WanderAmplitude  = FMath::FRandRange(0.15f, 0.65f);
    M->ChargeTimer      = FMath::FRandRange(2.f, 8.f);  // 동시 돌진 방지
    M->SetActorScale3D(FVector(FMath::FRandRange(0.7f, 1.4f)));

    // 비행형: 웨이브 2부터 30% 확률, 기지 900uu 위 목표, 속도+80, 시안색
    if (WaveIndex >= 1 && FMath::FRand() < 0.3f)
    {
        M->Variant    = EMonsterVariant::Flying;
        M->MoveSpeed += 80.f;
        M->TargetLoc  = Base ? Base->GetActorLocation() + FVector(0, 0, 900.f) : M->TargetLoc;
    }

    AliveMonsters++;
}

FVector ANeonGameMode::GetSpawnLocation(ESpawnDir Dir) const
{
    FVector Center = Base ? Base->GetActorLocation() : FVector::ZeroVector;

    for (ASpawnPoint* SP : SpawnPoints)
        if (SP && SP->Direction == Dir)
            return SP->GetActorLocation();

    // Fallback: offset from center
    switch (Dir)
    {
    case ESpawnDir::North: return Center + FVector( SpawnDistance, 0,           300.f);
    case ESpawnDir::East:  return Center + FVector( 0,             SpawnDistance,300.f);
    case ESpawnDir::South: return Center + FVector(-SpawnDistance, 0,           300.f);
    case ESpawnDir::West:  return Center + FVector( 0,            -SpawnDistance,300.f);
    }
    return Center;
}

void ANeonGameMode::RefreshAutoTurrets()
{
    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    int32 Count = GI ? GI->GetAutoTurretCount() : 1;
    for (int32 i = 0; i < AutoTurrets.Num(); i++)
        if (AutoTurrets[i]) AutoTurrets[i]->SetEnabled(i < Count);
}

void ANeonGameMode::SpawnResourceField()
{
    FVector Center = Base ? Base->GetActorLocation() : FVector::ZeroVector;

    float TotalWeight = 0.f;
    for (const FBlockDef& BD : BlockTable) TotalWeight += BD.SpawnWeight;
    if (TotalWeight <= 0.f || BlockTable.Num() == 0) return;

    for (int32 i = 0; i < TotalBlocksToSpawn; i++)
    {
        // Weighted random block type
        float Roll = FMath::FRand() * TotalWeight;
        float Acc  = 0.f;
        const FBlockDef* Chosen = &BlockTable[0];
        for (const FBlockDef& BD : BlockTable)
        {
            Acc += BD.SpawnWeight;
            if (Roll <= Acc) { Chosen = &BD; break; }
        }

        // Random ring position — area-uniform radius so blocks don't cluster at inner ring
        float Angle  = FMath::FRandRange(0.f, 2.f * PI);
        float MinR2  = BlockSpawnMinRadius * BlockSpawnMinRadius;
        float MaxR2  = BlockSpawnRadius    * BlockSpawnRadius;
        float Radius = FMath::Sqrt(FMath::FRand() * (MaxR2 - MinR2) + MinR2);
        float Z      = Center.Z + BlockSpawnHeight + FMath::FRandRange(-200.f, 600.f);
        FVector Loc  = FVector(Center.X + FMath::Cos(Angle) * Radius, 
                                Center.Y + FMath::Sin(Angle) * Radius, Z);

        AResourceBlock* Block = GetWorld()->SpawnActor<AResourceBlock>(
            AResourceBlock::StaticClass(), Loc, FRotator::ZeroRotator);
        if (Block) Block->InitFromDef(*Chosen);
    }
}

void ANeonGameMode::InitDefaultTables()
{
    if (WaveTable.Num() == 0)
    {
        auto Make = [](TArray<ESpawnDir> Dirs, int32 Num, float HP,
                       float Interval, float Speed, float DPS) -> FWaveDef
        {
            FWaveDef W;
            W.Entrances     = Dirs;
            W.TotalMonsters = Num;
            W.MonsterHP     = HP;
            W.SpawnInterval = Interval;
            W.MoveSpeed     = Speed;
            W.AttackDPS     = DPS;
            return W;
        };
        //                    Dirs                                                  Num  HP    Intv  Spd   DPS
        WaveTable.Add(Make({ESpawnDir::North},                                      5,  5.f,  2.0f, 350.f, 1.0f));
        WaveTable.Add(Make({ESpawnDir::North},                                     10,  5.f,  1.5f, 400.f, 1.0f));
        WaveTable.Add(Make({ESpawnDir::North, ESpawnDir::East},                    15, 10.f,  1.0f, 420.f, 1.5f));
        WaveTable.Add(Make({ESpawnDir::North, ESpawnDir::East, ESpawnDir::South},  20, 10.f,  1.0f, 450.f, 2.0f));
        WaveTable.Add(Make({ESpawnDir::North, ESpawnDir::East,
                            ESpawnDir::South, ESpawnDir::West},                    30, 15.f,  0.8f, 500.f, 2.5f));
    }

    if (BlockTable.Num() == 0)
    {
        auto MB = [](EBlockType T, FLinearColor C, int32 Pwr, float HP,
                        int32 SMin, int32 SMax, int32 SVal, float W) -> FBlockDef
        {
            FBlockDef D; D.Type=T; D.EmissiveColor=C; D.RequiredPower=Pwr;
            D.HP=HP; D.ShardMin=SMin; D.ShardMax=SMax; D.ShardValue=SVal; D.SpawnWeight=W;
            return D;
        };
        BlockTable.Add(MB(EBlockType::Gold,    FLinearColor(1.f,0.5f,0.f),   1,  5.f,  5,  8, 1, 6.f));
        BlockTable.Add(MB(EBlockType::Iron,    FLinearColor(0.f,1.f,1.f),    2,  8.f,  3,  5, 1, 3.f));
        BlockTable.Add(MB(EBlockType::Diamond, FLinearColor(1.f,1.f,1.f),    3, 12.f, 10, 15, 2, 1.f));
    }

    if (UpgradeTable.Num() == 0)
    {
        auto MU = [](EUpgradeId Id, const TCHAR* Name, const TCHAR* Desc, int32 Cost, int32 Max) -> FUpgradeDef
        {
            FUpgradeDef U; U.Id=Id; U.DisplayName=Name; U.EffectDesc=Desc; U.Cost=Cost; U.MaxLevel=Max;
            return U;
        };
        UpgradeTable.Add(MU(EUpgradeId::Ship_Magnet,      TEXT("Magnet Range"),    TEXT("+0.5m per lvl"),   12, 6));
        UpgradeTable.Add(MU(EUpgradeId::Ship_Attack,      TEXT("Ship Attack"),     TEXT("+1 DMG/Power"),    15, 4));
        UpgradeTable.Add(MU(EUpgradeId::Ship_Speed,       TEXT("Ship Speed"),      TEXT("+10m/s"),          18, 4));
        UpgradeTable.Add(MU(EUpgradeId::Ship_HP,          TEXT("Ship HP"),         TEXT("+2 HP"),           20, 4));
        UpgradeTable.Add(MU(EUpgradeId::Turret_Rotate,    TEXT("Turret RotSpeed"), TEXT("+90deg/s"),        20, 3));
        UpgradeTable.Add(MU(EUpgradeId::Turret_Attack,    TEXT("Turret Attack"),   TEXT("+1 DMG"),          15, 4));
        UpgradeTable.Add(MU(EUpgradeId::Turret_FireRate,  TEXT("Turret FireRate"), TEXT("+1 shot/s"),       15, 5));
        UpgradeTable.Add(MU(EUpgradeId::AutoTurret_Count, TEXT("Auto Turret +1"),  TEXT("+1 turret"),       30, 3));
    }
}
