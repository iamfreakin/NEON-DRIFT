#include "ManualTurret.h"
#include "Monster.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AManualTurret::AManualTurret()
{
    PrimaryActorTick.bCanEverTick = true;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
    RootComponent = BaseMesh;
    BaseMesh->SetCollisionProfileName(TEXT("NoCollision"));

    BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
    BarrelMesh->SetupAttachment(RootComponent);

    Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(BarrelMesh);
    Muzzle->SetRelativeLocation(FVector(150.f, 0, 0));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        BaseMesh->SetStaticMesh(CubeMesh.Object);
        BarrelMesh->SetStaticMesh(CubeMesh.Object);
    }
    BaseMesh->SetWorldScale3D(FVector(1.5f, 1.5f, 0.5f));
    BarrelMesh->SetRelativeScale3D(FVector(2.f, 0.3f, 0.3f));
    BarrelMesh->SetRelativeLocation(FVector(0, 0, 60));

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
    {
        BaseMesh->SetMaterial(0, NeonMat.Object);
        BarrelMesh->SetMaterial(0, NeonMat.Object);
    }

    TurretCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TurretCamera"));
    TurretCamera->SetupAttachment(RootComponent);
    TurretCamera->bAutoActivate = false;

    Stats.RotationSpeed = 30.f;
    Stats.FireRate       = 5.f;
    Stats.AttackDamage   = 1.f;
    Stats.Range          = 6000.f;
}

void AManualTurret::BeginPlay()
{
    Super::BeginPlay();
    FLinearColor Yellow(1.f, 0.85f, 0.f);
    auto SetNeon = [&](UStaticMeshComponent* M) {
        if (!M) return;
        UMaterialInstanceDynamic* MID = M->CreateAndSetMaterialInstanceDynamic(0);
        if (MID) { MID->SetVectorParameterValue(TEXT("BaseColor"), Yellow); MID->SetScalarParameterValue(TEXT("Glow"), 3.f); }
    };
    SetNeon(BaseMesh);
    SetNeon(BarrelMesh);
}

void AManualTurret::SetPlayerBoarded(bool b)
{
    bPlayerBoarded = b;
    if (b)
    {
        TurretCamera->Activate();
        if (BoardSound) UGameplayStatics::SpawnSoundAtLocation(this, BoardSound, GetActorLocation());
    }
    else
    {
        TurretCamera->Deactivate();
        FireCooldown = 0.f;
        if (UnboardSound) UGameplayStatics::SpawnSoundAtLocation(this, UnboardSound, GetActorLocation());
    }
}

void AManualTurret::ApplyStats(const FTurretStats& InStats)
{
    Stats = InStats;
}

void AManualTurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bPlayerBoarded) return;

    // Float camera behind turret in world space — bypasses component scale issues
    FVector AimDir = DesiredAim.Vector();
    FVector CamLoc = GetActorLocation() - AimDir * 500.f + FVector(0.f, 0.f, 220.f);
    TurretCamera->SetWorldLocationAndRotation(CamLoc, DesiredAim);

    // Barrel rotates toward aim with speed limit (core upgrade mechanic)
    FRotator Current = BarrelMesh->GetComponentRotation();
    FRotator New     = FMath::RInterpConstantTo(Current, DesiredAim, DeltaTime, Stats.RotationSpeed);
    BarrelMesh->SetWorldRotation(FRotator(New.Pitch, New.Yaw, 0));

    FireCooldown -= DeltaTime;
}

void AManualTurret::Fire()
{
    // Hit detection: from camera (screen center) in barrel direction
    FVector TraceStart = TurretCamera->GetComponentLocation();
    FVector TraceEnd   = TraceStart + BarrelMesh->GetForwardVector() * Stats.Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
    FVector HitPoint = bHit ? Hit.ImpactPoint : TraceEnd;

    // Visual tracer: from muzzle to hit point
    DrawDebugLine(GetWorld(), Muzzle->GetComponentLocation(), HitPoint, FColor::Orange, false, 0.05f, 0, 2.f);
    if (bHit)
    {
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 20.f, 6, FColor::Orange, false, 0.15f);
        if (IDamageable* D = Cast<IDamageable>(Hit.GetActor()))
            D->TakeHit(Stats.AttackDamage, 99);
    }
    if (FireSound) UGameplayStatics::SpawnSoundAtLocation(this, FireSound, GetActorLocation());
}
