#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "Sound/SoundBase.h"
#include "AutoTurret.generated.h"

class AMonster;

UCLASS()
class NEONDRIFT_API AAutoTurret : public AActor
{
    GENERATED_BODY()
public:
    AAutoTurret();

    bool bEnabled = false;

    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* FireSound = nullptr;

    void SetEnabled(bool bEnable);
    void ApplyStats(const FTurretStats& InStats);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY() UStaticMeshComponent*      Mesh = nullptr;
    UPROPERTY() USceneComponent*          Barrel = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* MID    = nullptr;
    UPROPERTY() TWeakObjectPtr<AMonster> Target;

    FTurretStats Stats;
    float FireCooldown = 0.f;

    void FindTarget();
    void FireAtTarget();
};
