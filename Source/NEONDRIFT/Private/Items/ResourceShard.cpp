#include "ResourceShard.h"
#include "PlayerShip.h"
#include "NeonGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

AResourceShard::AResourceShard()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("NoCollision"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
        Mesh->SetStaticMesh(SphereMesh.Object);

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
        Mesh->SetMaterial(0, NeonMat.Object);

    Mesh->SetWorldScale3D(FVector(0.15f));
}

void AResourceShard::BeginPlay()
{
    Super::BeginPlay();
    UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (MID)
    {
        MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.75f, 0.f)); // Gold
        MID->SetScalarParameterValue(TEXT("Glow"), 4.f);
    }
}

void AResourceShard::AttractTo(APlayerShip* Ship)
{
    HomingTarget = Ship;
}

void AResourceShard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APlayerShip* Ship = HomingTarget.Get();
    if (!Ship) return;

    FVector ToShip = Ship->GetActorLocation() - GetActorLocation();
    float Dist = ToShip.Size();

    // Give up homing if ship has moved too far away
    if (Dist > 1200.f)
    {
        HomingTarget = nullptr;
        Velocity = FVector::ZeroVector;
        return;
    }

    if (Dist < 80.f)
    {
        ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
        if (GM) GM->AddResources(Value); // GameMode plays ShardCollectSound
        Destroy();
        return;
    }

    // Kill tangential (sideways) velocity so shards fly straight instead of orbiting
    FVector Dir = ToShip.GetSafeNormal();
    float   RadialSpeed = FVector::DotProduct(Velocity, Dir);
    FVector Tangential  = Velocity - Dir * RadialSpeed;
    Velocity = Dir * RadialSpeed + Tangential * FMath::Exp(-10.f * DeltaTime);

    // Accelerate toward ship, capped at 2500 uu/s
    Velocity += Dir * HomingAccel * DeltaTime;
    if (Velocity.Size() > 2500.f) Velocity = Velocity.GetSafeNormal() * 2500.f;

    AddActorWorldOffset(Velocity * DeltaTime);
}
