#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "ManualTurret.generated.h"

class UCameraComponent;

UCLASS()
class NEONDRIFT_API AManualTurret : public AActor
{
    GENERATED_BODY()
public:
    AManualTurret();

    FRotator DesiredAim;      // set each tick by PlayerController
    bool     bPlayerBoarded = false;

    void SetPlayerBoarded(bool b);
    void ApplyStats(const FTurretStats& InStats);
    void Fire(); // called by PlayerShip when boarded + LMB

    FVector GetBarrelForward()  const { return BarrelMesh ? BarrelMesh->GetForwardVector() : FVector::ForwardVector; }
    FVector GetMuzzleLocation() const { return Muzzle    ? Muzzle->GetComponentLocation() : GetActorLocation(); }

    UPROPERTY(EditAnywhere) float BoardingRadius = 800.f;
    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* FireSound    = nullptr;
    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* BoardSound   = nullptr;
    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* UnboardSound = nullptr;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY() UStaticMeshComponent* BaseMesh    = nullptr;
    UPROPERTY() UStaticMeshComponent* BarrelMesh  = nullptr;
    UPROPERTY() USceneComponent*      Muzzle      = nullptr;
    UPROPERTY() UCameraComponent*     TurretCamera= nullptr;

    FTurretStats Stats;
    float FireCooldown = 0.f;
};
