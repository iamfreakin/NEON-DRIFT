#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "ResourceBlock.generated.h"

UCLASS()
class NEONDRIFT_API AResourceBlock : public AActor, public IDamageable
{
    GENERATED_BODY()
public:
    AResourceBlock();

    void InitFromDef(const FBlockDef& Def);

    virtual void TakeHit(float Damage, int32 AttackerPower) override;

private:
    UPROPERTY() UStaticMeshComponent* Mesh    = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* MID = nullptr;

    int32 RequiredPower = 1;
    float HP            = 3.f;
    int32 ShardMin      = 5;
    int32 ShardMax      = 8;
    int32 ShardValue    = 1;
    FLinearColor EmissiveColor = FLinearColor(1.f, 0.5f, 0.f);

    void DropShards();
    void SpawnSpark(); // small visual feedback when attack power is too low
};
