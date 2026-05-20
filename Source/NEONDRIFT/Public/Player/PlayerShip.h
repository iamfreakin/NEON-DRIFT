#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NeonTypes.h"
#include "InputActionValue.h"
#include "Sound/SoundBase.h"
#include "PlayerShip.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class NEONDRIFT_API APlayerShip : public APawn, public IDamageable
{
    GENERATED_BODY()
public:
    APlayerShip();

    FShipStats Stats;
    float CurrentHP = 3.f;

    void ApplyStats(const FShipStats& InStats);

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    virtual void TakeHit(float Damage, int32 AttackerPower) override;

    // Input handlers (called by EIC bindings)
    void OnMoveForward(const FInputActionValue& Value);
    void OnMoveRight  (const FInputActionValue& Value);
    void OnThrust     (const FInputActionValue& Value);
    void OnLook       (const FInputActionValue& Value);
    void StartFire    (const FInputActionValue& Value);
    void StopFire     (const FInputActionValue& Value);
    void OnReady      (const FInputActionValue& Value);

    UPROPERTY() UStaticMeshComponent*          Mesh      = nullptr;
    UPROPERTY() USpringArmComponent*           SpringArm = nullptr;
    UPROPERTY() UCameraComponent*              Camera    = nullptr;
    UPROPERTY() USceneComponent*               Muzzle    = nullptr;
    UPROPERTY() UInstancedStaticMeshComponent* TrailISMC = nullptr;
    UPROPERTY() UMaterialInstanceDynamic*      ShipMID   = nullptr;

    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* FireSound   = nullptr;
    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* HitSound    = nullptr;
    UPROPERTY(EditAnywhere, Category="Audio") USoundBase* EngineSound = nullptr;
    UPROPERTY() UAudioComponent* EngineAudio = nullptr;

private:
    FVector Velocity = FVector::ZeroVector;
    float   MoveForwardInput = 0.f;
    float   MoveRightInput   = 0.f;
    float   ThrustInput      = 0.f;
    bool    bFiring          = false;
    float   FireCooldown     = 0.f;

    TArray<FVector> TrailPos;
    TArray<FQuat>   TrailRot;
    TArray<float>   TrailAge;
    float           TrailSampleTimer = 0.f;

    void FireOnce();
    void CollectShards();
    void UpdateTrail(float DeltaTime);

};
