#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "AutoTurret.generated.h"

class AMonster;

UCLASS()
class NEONDRIFT_API AAutoTurret : public AActor
{
    GENERATED_BODY()
public:
    AAutoTurret();

    bool bEnabled = false;

    void SetEnabled(bool bEnable);
    void ApplyStats(const FTurretStats& InStats);

    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY() UStaticMeshComponent* Mesh   = nullptr;
    UPROPERTY() USceneComponent*      Barrel = nullptr;
    UPROPERTY() TWeakObjectPtr<AMonster> Target;

    FTurretStats Stats;
    float FireCooldown = 0.f;

    void FindTarget();
    void FireAtTarget();
};
