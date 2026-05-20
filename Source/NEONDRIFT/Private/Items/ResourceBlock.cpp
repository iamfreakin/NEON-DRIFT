#include "ResourceBlock.h"
#include "ResourceShard.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

AResourceBlock::AResourceBlock()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
        Mesh->SetStaticMesh(CubeMesh.Object);

    SetActorScale3D(FVector(1.5f));
}

void AResourceBlock::InitFromDef(const FBlockDef& Def)
{
    RequiredPower  = Def.RequiredPower;
    HP             = Def.HP;
    ShardMin       = Def.ShardMin;
    ShardMax       = Def.ShardMax;
    ShardValue     = Def.ShardValue;
    EmissiveColor  = Def.EmissiveColor;

    MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (MID)
    {
        MID->SetVectorParameterValue(TEXT("BaseColor"), EmissiveColor);
        MID->SetScalarParameterValue(TEXT("Glow"), 3.f);
    }
}

void AResourceBlock::TakeHit(float Damage, int32 AttackerPower)
{
    if (AttackerPower < RequiredPower)
    {
        SpawnSpark();
        return; // attack power gate
    }

    HP -= Damage;
    if (HP <= 0.f)
    {
        DropShards();
        Destroy();
    }
}

void AResourceBlock::DropShards()
{
    int32 Count = FMath::RandRange(ShardMin, ShardMax);
    for (int32 i = 0; i < Count; i++)
    {
        FVector Offset = FMath::VRand() * FMath::FRandRange(50.f, 200.f);
        AResourceShard* Shard = GetWorld()->SpawnActor<AResourceShard>(
            AResourceShard::StaticClass(),
            GetActorLocation() + Offset,
            FRotator::ZeroRotator);
        if (Shard) Shard->Value = ShardValue;
    }
}

void AResourceBlock::SpawnSpark()
{
    // Visual: orange sphere = "power too low" feedback
    DrawDebugSphere(GetWorld(), GetActorLocation(), 80.f, 8, FColor::Orange, false, 0.3f, 0, 3.f);
}
