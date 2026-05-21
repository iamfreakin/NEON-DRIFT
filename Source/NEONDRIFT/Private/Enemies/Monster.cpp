#include "Monster.h"
#include "DrawDebugHelpers.h"
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
}

void AMonster::BeginPlay()
{
    Super::BeginPlay();
    MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
}

void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Color is assigned by GameMode after SpawnActor, so initialize on first Tick
    if (!bMIDInitialized && MID)
    {
        bMIDInitialized = true;
        FLinearColor C = (Variant == EMonsterVariant::Flying)
            ? FLinearColor(0.f, 1.f, 1.f)   // 시안형 비행형
            : FLinearColor(1.f, 0.25f, 0.f); // 오렌지색 지상형
        MID->SetVectorParameterValue(TEXT("BaseColor"), C);
        MID->SetScalarParameterValue(TEXT("Glow"), 3.f);
    }

    ABase* Base = BaseRef.Get();
    if (!Base) return;

    float DistToBase = FVector::Dist(GetActorLocation(), Base->GetActorLocation());

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

        // 돌진 시스템
        ChargeDuration -= DeltaTime;
        ChargeTimer    -= DeltaTime;
        if (ChargeTimer <= 0.f && ChargeDuration <= 0.f)
        {
            ChargeDuration = FMath::FRandRange(0.8f, 1.5f);
            ChargeTimer    = FMath::FRandRange(3.f, 7.f);
        }
        bool bCharging = ChargeDuration > 0.f;

        // A: 비행형/돌진형은 기지로 직진 (돌진 중 흔들림 제거)
        FVector BaseLoc    = Base->GetActorLocation();
        FVector MoveTarget = (!bCharging && DistToBase > AttackRange * 1.5f) ? BaseLoc + TargetOffset : BaseLoc;
        FVector Dir        = (MoveTarget - GetActorLocation()).GetSafeNormal();

        // B: 횡방향 사인파 흔들림 (돌진 중 중단)
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

