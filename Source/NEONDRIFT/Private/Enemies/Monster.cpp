#include "Monster.h"
#include "NeonBase.h"
#include "NeonGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AMonster::AMonster()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("OverlapAll"));
    Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
        Mesh->SetStaticMesh(CubeMesh.Object);

    SetActorScale3D(FVector(1.f)); // 100uu cube
}

void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ABase* Base = BaseRef.Get();
    if (!Base) return;

    float DistToBase = FVector::Dist(GetActorLocation(), Base->GetActorLocation());

    if (DistToBase <= AttackRange)
    {
        // In range: stop and deal ranged DPS
        Base->TakeHit(AttackDPS * DeltaTime, 0);
        bAttacking = true;
    }
    else
    {
        bAttacking = false;
        FVector Dir = (Base->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        AddActorWorldOffset(Dir * MoveSpeed * DeltaTime, true);
    }
}

void AMonster::TakeHit(float Damage, int32 /*AttackerPower*/)
{
    HP -= Damage;
    if (HP <= 0.f)
    {
        ANeonGameMode* GM = GameModeRef.Get();
        if (GM) GM->OnMonsterKilled();
        Destroy();
    }
}
