#include "Monster.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NeonBase.h"
#include "NeonGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

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

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
        Mesh->SetMaterial(0, NeonMat.Object);

    SetActorScale3D(FVector(1.f));

    // Monster cube scale (1,1,1) → half-extents: ±50
    TrailFX_BL = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX_BL"));
    TrailFX_BL->SetupAttachment(RootComponent);
    TrailFX_BL->SetRelativeLocation(FVector(-50,  50, 0));
    TrailFX_BL->bAutoActivate = false;

    TrailFX_BR = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX_BR"));
    TrailFX_BR->SetupAttachment(RootComponent);
    TrailFX_BR->SetRelativeLocation(FVector(-50, -50, 0));
    TrailFX_BR->bAutoActivate = false;
}

void AMonster::BeginPlay()
{
    Super::BeginPlay();
    MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (UNiagaraSystem* NS = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/FX/NS_NeonTrail.NS_NeonTrail")))
    {
        TArray<UNiagaraComponent*> Trails = {TrailFX_BL, TrailFX_BR};
        for (UNiagaraComponent* T : Trails)
        {
            T->SetAsset(NS);
            T->Activate(true);
        }
    }
}

void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Variant is assigned by GameMode after SpawnActor, so init on first Tick
    if (!bMIDInitialized)
    {
        bMIDInitialized = true;

        if (Variant == EMonsterVariant::Flying)
        {
            if (UStaticMesh* ConeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone")))
                Mesh->SetStaticMesh(ConeMesh);
            Mesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f)); // tip → +X (forward)
            MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
        }

        if (MID)
        {
            FLinearColor C = (Variant == EMonsterVariant::Flying)
                ? FLinearColor(0.f, 1.f, 1.f)
                : FLinearColor(1.f, 0.25f, 0.f);
            MID->SetVectorParameterValue(TEXT("BaseColor"), C);
            MID->SetScalarParameterValue(TEXT("Glow"), 3.f);

            TArray<UNiagaraComponent*> Trails = {TrailFX_BL, TrailFX_BR};
            for (UNiagaraComponent* T : Trails)
                if (T) T->SetColorParameter(FName("TrailColor"), C);
        }
    }

    ABase* Base = BaseRef.Get();
    if (!Base) return;

    float DistToBase = FVector::Dist(GetActorLocation(), Base->GetActorLocation());

    // ── Flying: orbital spiral ────────────────────────────────
    if (Variant == EMonsterVariant::Flying)
    {
        if (!bOrbitInitialized)
        {
            bOrbitInitialized = true;
            FVector Flat = GetActorLocation() - Base->GetActorLocation();
            Flat.Z = 0.f;
            OrbitRadius = FMath::Max(AttackRange, Flat.Size());
            OrbitAngle  = FMath::Atan2(Flat.Y, Flat.X);
        }

        if (DistToBase <= AttackRange)
        {
            bAttacking = true;
            Base->TakeHit(AttackDPS * DeltaTime, 0);
            AttackSoundCooldown -= DeltaTime;
            if (AttackSoundCooldown <= 0.f)
            {
                AttackSoundCooldown = 1.5f;
                DrawDebugLine(GetWorld(), GetActorLocation(), Base->GetActorLocation(), FColor::Red, false, 0.3f, 0, 2.f);
                if (ANeonGameMode* GM = GameModeRef.Get())
                    if (GM->MonsterAttackSound)
                        UGameplayStatics::SpawnSoundAtLocation(this, GM->MonsterAttackSound, GetActorLocation());
            }
        }
        else
        {
            bAttacking = false;
            OrbitRadius = FMath::Max(AttackRange * 0.6f, OrbitRadius - 100.f * DeltaTime);
            OrbitAngle += (MoveSpeed / FMath::Max(OrbitRadius, 1.f)) * DeltaTime;

            FVector BaseLoc = Base->GetActorLocation();
            FVector Target(
                BaseLoc.X + FMath::Cos(OrbitAngle) * OrbitRadius,
                BaseLoc.Y + FMath::Sin(OrbitAngle) * OrbitRadius,
                TargetLoc.Z
            );

            FVector MoveDir = (Target - GetActorLocation()).GetSafeNormal();
            AddActorWorldOffset(MoveDir * MoveSpeed * DeltaTime, true);

            if (!MoveDir.IsNearlyZero())
                SetActorRotation(FMath::RInterpTo(GetActorRotation(), MoveDir.Rotation(), DeltaTime, 8.f));
        }
        return;
    }

    // ── Ground: existing logic ────────────────────────────────
    if (DistToBase <= AttackRange)
    {
        Base->TakeHit(AttackDPS * DeltaTime, 0);
        bAttacking = true;

        AttackSoundCooldown -= DeltaTime;
        if (AttackSoundCooldown <= 0.f)
        {
            AttackSoundCooldown = 1.5f;
            DrawDebugLine(GetWorld(), GetActorLocation(), Base->GetActorLocation(), FColor::Red, false, 0.3f, 0, 2.f);
            if (ANeonGameMode* GM = GameModeRef.Get())
                if (GM->MonsterAttackSound)
                    UGameplayStatics::SpawnSoundAtLocation(this, GM->MonsterAttackSound, GetActorLocation());
        }
    }
    else
    {
        bAttacking = false;

        ChargeDuration -= DeltaTime;
        ChargeTimer    -= DeltaTime;
        if (ChargeTimer <= 0.f && ChargeDuration <= 0.f)
        {
            ChargeDuration = FMath::FRandRange(0.8f, 1.5f);
            ChargeTimer    = FMath::FRandRange(3.f, 7.f);
        }
        bool bCharging = ChargeDuration > 0.f;

        FVector BaseLoc    = Base->GetActorLocation();
        FVector MoveTarget = (!bCharging && DistToBase > AttackRange * 1.5f) ? BaseLoc + TargetOffset : BaseLoc;
        FVector Dir        = (MoveTarget - GetActorLocation()).GetSafeNormal();

        if (!bCharging)
        {
            WanderTime += DeltaTime;
            FVector Lateral = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal();
            Dir = (Dir + Lateral * FMath::Sin(WanderTime * WanderFreq + WanderPhase) * WanderAmplitude).GetSafeNormal();
        }

        float ActualSpeed = bCharging ? MoveSpeed * 2.5f : MoveSpeed;
        AddActorWorldOffset(Dir * ActualSpeed * DeltaTime, true);
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

