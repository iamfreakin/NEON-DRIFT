#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "Monster.generated.h"

class ABase;
class ANeonGameMode;
class UNiagaraComponent;

UCLASS()
class NEONDRIFT_API AMonster : public AActor, public IDamageable
{
    GENERATED_BODY()
public:
    AMonster();

    // Set by WaveManager before spawn
    float HP         = 1.f;
    float MaxHP      = 1.f;
    float MoveSpeed  = 1000.f;
    float AttackDPS  = 1.f;
    EMonsterColor   Color   = EMonsterColor::Orange;
    EMonsterVariant Variant = EMonsterVariant::Ground;
    FVector TargetLoc   = FVector::ZeroVector;

    UPROPERTY() TWeakObjectPtr<ABase>         BaseRef;
    UPROPERTY() TWeakObjectPtr<ANeonGameMode> GameModeRef;

    FVector TargetOffset    = FVector::ZeroVector;
    float   WanderPhase     = 0.f;
    float   WanderFreq      = 1.f;
    float   WanderAmplitude = 0.4f;
    float   WanderTime      = 0.f;
    float   ChargeTimer     = 5.f;
    float   ChargeDuration  = 0.f;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void TakeHit(float Damage, int32 AttackerPower) override;

private:
    UPROPERTY() UStaticMeshComponent*     Mesh    = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* MID     = nullptr;
    UPROPERTY() UNiagaraComponent* TrailFX_BL = nullptr;
    UPROPERTY() UNiagaraComponent* TrailFX_BR = nullptr;
    bool  bAttacking          = false;
    bool  bMIDInitialized     = false;
    float AttackRange         = 2000.f;
    float AttackSoundCooldown = 0.f;

    // Flying orbit state
    float OrbitAngle        = 0.f;
    float OrbitRadius       = 0.f;
    bool  bOrbitInitialized = false;
};
