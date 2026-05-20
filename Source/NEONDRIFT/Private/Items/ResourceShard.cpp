#include "ResourceShard.h"
#include "PlayerShip.h"
#include "NeonGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
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

    Mesh->SetWorldScale3D(FVector(0.15f));
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
        if (GM) GM->AddResources(Value);
        Destroy();
        return;
    }

    // Accelerate toward ship, capped at 2500 uu/s
    Velocity += ToShip.GetSafeNormal() * HomingAccel * DeltaTime;
    if (Velocity.Size() > 2500.f) Velocity = Velocity.GetSafeNormal() * 2500.f;

    AddActorWorldOffset(Velocity * DeltaTime);
}
