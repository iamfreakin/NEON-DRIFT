#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceShard.generated.h"

class APlayerShip;

UCLASS()
class NEONDRIFT_API AResourceShard : public AActor
{
    GENERATED_BODY()
public:
    AResourceShard();

    UPROPERTY() int32 Value = 1;

    void AttractTo(APlayerShip* Ship);

    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY() TWeakObjectPtr<APlayerShip> HomingTarget;
    UPROPERTY() UStaticMeshComponent* Mesh = nullptr;

    float HomingAccel = 2000.f;
    FVector Velocity  = FVector::ZeroVector;
};
