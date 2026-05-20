#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "ManualTurret.generated.h"

UCLASS()
class NEONDRIFT_API AManualTurret : public AActor
{
    GENERATED_BODY()
public:
    AManualTurret();

    FRotator DesiredAim; // set each tick by PlayerController
    bool     bActive = false;

    void SetActive(bool b);
    void ApplyStats(const FTurretStats& InStats);

    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY() UStaticMeshComponent* BaseMesh   = nullptr;
    UPROPERTY() UStaticMeshComponent* BarrelMesh = nullptr;
    UPROPERTY() USceneComponent*      Muzzle     = nullptr;

    FTurretStats Stats;
    float FireCooldown = 0.f;

    void Fire();
};
