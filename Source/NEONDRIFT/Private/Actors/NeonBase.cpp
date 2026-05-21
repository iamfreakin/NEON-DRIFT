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

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
        Mesh->SetMaterial(0, NeonMat.Object);

    Mesh->SetWorldScale3D(FVector(1.0f));
    SetActorScale3D(FVector(10.f));
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

    if (!MID) return;

    // Decay hit flash
    if (HitFlashTimer > 0.f)
    {
        HitFlashTimer -= DeltaTime * 4.f;
        float T = FMath::Max(0.f, HitFlashTimer);
        FLinearColor Flash = FMath::Lerp(FLinearColor::White, FLinearColor::Red, T);
        if (GetHPFraction() <= 0.25f)
            Flash = FMath::Lerp(FLinearColor(1.f, 0.4f, 0.f), FLinearColor::Red, T);
        MID->SetVectorParameterValue(TEXT("BaseColor"), Flash);
    }
    else
    {
        FLinearColor Base = GetHPFraction() <= 0.25f ? FLinearColor(1.f, 0.4f, 0.f) : FLinearColor::White;
        MID->SetVectorParameterValue(TEXT("BaseColor"), Base);
    }

    // Neon emissive pulse
    float Glow = 3.f + 1.5f * FMath::Sin(GetGameTimeSinceCreation() * 2.f);
    MID->SetScalarParameterValue(TEXT("Glow"), Glow);
}

void ABase::TakeHit(float Damage, int32 /*AttackerPower*/)
{
    if (GameModeRef && GameModeRef->Phase == EGamePhase::PreWave) return;

    CurrentHP = FMath::Max(0.f, CurrentHP - Damage);
    HitFlashTimer = 1.f;

    if (HitSound) UGameplayStatics::SpawnSoundAtLocation(this, HitSound, GetActorLocation());
    if (HitShakeClass)
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
            PC->ClientStartCameraShake(HitShakeClass, 0.1f);

    // 25% HP 경고음 (1회)
    if (!bLowHPSoundPlayed && GetHPFraction() <= 0.25f && CurrentHP > 0.f)
    {
        bLowHPSoundPlayed = true;
        if (LowHPSound) UGameplayStatics::PlaySound2D(this, LowHPSound);
    }

    if (CurrentHP <= 0.f && GameModeRef)
    {
        if (DestroySound) UGameplayStatics::SpawnSoundAtLocation(this, DestroySound, GetActorLocation());
        GameModeRef->OnBaseDestroyed();
    }
}
