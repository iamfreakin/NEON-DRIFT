#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "NeonTypes.h"
#include "NeonBase.generated.h"

class ANeonGameMode;

UCLASS()
class NEONDRIFT_API ABase : public AActor, public IDamageable
{
    GENERATED_BODY()
public:
    ABase();

    UPROPERTY(EditAnywhere) float MaxHP = 5.f;
    UPROPERTY(EditAnywhere) TSubclassOf<UCameraShakeBase> HitShakeClass;

    UPROPERTY() ANeonGameMode* GameModeRef = nullptr;

    float CurrentHP = 5.f;
    float HitFlashTimer = 0.f;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void TakeHit(float Damage, int32 AttackerPower) override;

    float GetHPFraction() const { return MaxHP > 0.f ? CurrentHP / MaxHP : 0.f; }

private:
    UPROPERTY() UStaticMeshComponent* Mesh    = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* MID = nullptr;
};
