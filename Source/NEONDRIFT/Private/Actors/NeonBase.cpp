#include "NeonBase.h"
#include "NeonGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

ABase::ABase()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
        Mesh->SetStaticMesh(CubeMesh.Object);

    Mesh->SetWorldScale3D(FVector(1.0f)); // 100uu = 1m cube, scaled to 10m in Tick or via Details
    SetActorScale3D(FVector(10.f)); // 10m x 10m x 10m
}

void ABase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
    MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
}

void ABase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decay hit flash
    if (HitFlashTimer > 0.f)
    {
        HitFlashTimer -= DeltaTime * 4.f;
        if (MID)
        {
            float T = FMath::Max(0.f, HitFlashTimer);
            // Flash red → normal white/orange on low HP
            FLinearColor Flash = FMath::Lerp(FLinearColor::White, FLinearColor::Red, T);
            if (GetHPFraction() <= 0.25f)
                Flash = FMath::Lerp(FLinearColor(1.f, 0.4f, 0.f), FLinearColor::Red, T); // orange warning
            MID->SetVectorParameterValue(TEXT("BaseColor"), Flash);
        }
    }
    else if (MID)
    {
        FLinearColor Base = GetHPFraction() <= 0.25f ? FLinearColor(1.f, 0.4f, 0.f) : FLinearColor::White;
        MID->SetVectorParameterValue(TEXT("BaseColor"), Base);
    }
}

void ABase::TakeHit(float Damage, int32 /*AttackerPower*/)
{
    CurrentHP = FMath::Max(0.f, CurrentHP - Damage);
    HitFlashTimer = 1.f;

    if (CurrentHP <= 0.f && GameModeRef)
    {
        GameModeRef->OnBaseDestroyed();
    }
}
