#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "Monster.generated.h"

class ABase;
class ANeonGameMode;

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
    EMonsterColor Color = EMonsterColor::Orange;
    FVector TargetLoc   = FVector::ZeroVector;

    UPROPERTY() TWeakObjectPtr<ABase>         BaseRef;
    UPROPERTY() TWeakObjectPtr<ANeonGameMode> GameModeRef;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void TakeHit(float Damage, int32 AttackerPower) override;

private:
    UPROPERTY() UStaticMeshComponent* Mesh = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* MID = nullptr;
    bool  bAttacking          = false;
    bool  bMIDInitialized     = false;
    float AttackRange         = 2000.f;
    float AttackSoundCooldown = 0.f;
};
