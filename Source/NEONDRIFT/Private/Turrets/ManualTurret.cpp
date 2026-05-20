#include "ManualTurret.h"
#include "Monster.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
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

    TurretCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TurretCamera"));
    TurretCamera->SetupAttachment(RootComponent);
    TurretCamera->bAutoActivate = false;

    Stats.RotationSpeed = 30.f;
    Stats.FireRate       = 5.f;
    Stats.AttackDamage   = 1.f;
    Stats.Range          = 6000.f;
}

void AManualTurret::SetPlayerBoarded(bool b)
{
    bPlayerBoarded = b;
    if (b)
        TurretCamera->Activate();
    else
    {
        TurretCamera->Deactivate();
        FireCooldown = 0.f;
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
    FVector Start = TurretCamera->GetComponentLocation();
    FVector End   = Start + BarrelMesh->GetForwardVector() * Stats.Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 20.f, 6, FColor::Orange, false, 0.15f);
        if (IDamageable* D = Cast<IDamageable>(Hit.GetActor()))
            D->TakeHit(Stats.AttackDamage, 99);
    }
}
