#include "AutoTurret.h"
#include "Monster.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AAutoTurret::AAutoTurret()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylMesh.Succeeded())
        Mesh->SetStaticMesh(CylMesh.Object);

    Barrel = CreateDefaultSubobject<USceneComponent>(TEXT("Barrel"));
    Barrel->SetupAttachment(RootComponent);

    SetActorScale3D(FVector(0.5f, 0.5f, 1.f));

    Stats.RotationSpeed = 120.f;
    Stats.FireRate       = 1.0f;
    Stats.AttackDamage   = 0.5f;
    Stats.Range          = 2500.f;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
        Mesh->SetMaterial(0, NeonMat.Object);
}

void AAutoTurret::BeginPlay()
{
    Super::BeginPlay();
    MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (MID)
    {
        MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 1.f, 0.35f)); // Green
        MID->SetScalarParameterValue(TEXT("Glow"), 3.f);
    }
}

void AAutoTurret::SetEnabled(bool bEnable)
{
    bEnabled = bEnable;
    SetActorHiddenInGame(!bEnable);
    SetActorTickEnabled(bEnable);
}

void AAutoTurret::ApplyStats(const FTurretStats& InStats)
{
    Stats = InStats;
}

void AAutoTurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bEnabled) return;

    // Reacquire dead/out-of-range target
    AMonster* T = Target.Get();
    if (!T || !IsValid(T) || FVector::Dist(GetActorLocation(), T->GetActorLocation()) > Stats.Range)
        FindTarget();

    T = Target.Get();
    if (!T) return;

    // Rotate toward target
    FVector Dir = (T->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator Desired = Dir.Rotation();
    FRotator Current = GetActorRotation();
    FRotator New = FMath::RInterpConstantTo(Current, Desired, DeltaTime, Stats.RotationSpeed);
    SetActorRotation(New);

    // Fire
    FireCooldown -= DeltaTime;
    if (FireCooldown <= 0.f)
    {
        FireAtTarget();
        FireCooldown = 1.f / FMath::Max(0.1f, Stats.FireRate);
    }
}

void AAutoTurret::FindTarget()
{
    AMonster* Best = nullptr;
    float BestDist = Stats.Range;

    for (TActorIterator<AMonster> It(GetWorld()); It; ++It)
    {
        float D = FVector::Dist(GetActorLocation(), It->GetActorLocation());
        if (D < BestDist) { BestDist = D; Best = *It; }
    }
    Target = Best;
}

void AAutoTurret::FireAtTarget()
{
    AMonster* T = Target.Get();
    if (!T) return;

    FHitResult Hit;
    FVector Start = GetActorLocation() + FVector(0, 0, 50);
    FVector End   = T->GetActorLocation();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (IDamageable* D = Cast<IDamageable>(Hit.GetActor()))
        {
            D->TakeHit(Stats.AttackDamage, 99);
            if (FireSound) UGameplayStatics::SpawnSoundAtLocation(this, FireSound, Start);
        }
    }
}
