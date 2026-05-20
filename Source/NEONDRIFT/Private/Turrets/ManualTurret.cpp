#include "ManualTurret.h"
#include "Monster.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

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

    Stats.RotationSpeed = 30.f;
    Stats.FireRate       = 5.f;
    Stats.AttackDamage   = 1.f;
    Stats.Range          = 6000.f;
}

void AManualTurret::SetActive(bool b)
{
    bActive = b;
    if (!b) FireCooldown = 0.f;
}

void AManualTurret::ApplyStats(const FTurretStats& InStats)
{
    Stats = InStats;
}

void AManualTurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bActive) return;

    // Rotate barrel toward DesiredAim at limited speed (KEY mechanic)
    FRotator Current = BarrelMesh->GetComponentRotation();
    FRotator New     = FMath::RInterpConstantTo(Current, DesiredAim, DeltaTime, Stats.RotationSpeed);
    BarrelMesh->SetWorldRotation(FRotator(New.Pitch, New.Yaw, 0));

    FireCooldown -= DeltaTime;
    if (FireCooldown <= 0.f)
    {
        Fire();
        FireCooldown = 1.f / FMath::Max(0.1f, Stats.FireRate);
    }
}

void AManualTurret::Fire()
{
    FVector Start = Muzzle->GetComponentLocation();
    FVector End   = Start + BarrelMesh->GetForwardVector() * Stats.Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (IDamageable* D = Cast<IDamageable>(Hit.GetActor()))
            D->TakeHit(Stats.AttackDamage, 99);
    }
}
