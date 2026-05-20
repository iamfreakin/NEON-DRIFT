#include "PlayerShip.h"
#include "NeonGameMode.h"
#include "NeonGameInstance.h"
#include "ResourceShard.h"
#include "NeonPlayerController.h"
#include "ManualTurret.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

APlayerShip::APlayerShip()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("Pawn"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
        Mesh->SetStaticMesh(CubeMesh.Object);
    Mesh->SetWorldScale3D(FVector(2.f, 1.f, 0.4f));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength         = 900.f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritPitch           = true;
    SpringArm->bInheritRoll            = false;
    SpringArm->SocketOffset            = FVector(0, 0, 60);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
    Camera->FieldOfView             = 80.f;

    Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(RootComponent);
    Muzzle->SetRelativeLocation(FVector(130.f, 0, 0));

    bUseControllerRotationYaw = false;
}

void APlayerShip::BeginPlay()
{
    Super::BeginPlay();
    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    if (GI) ApplyStats(GI->GetShipStats());
    CurrentHP = Stats.MaxHP;
}

void APlayerShip::ApplyStats(const FShipStats& InStats)
{
    Stats     = InStats;
    CurrentHP = FMath::Min(CurrentHP, Stats.MaxHP);
}

void APlayerShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    ANeonPlayerController* PC  = Cast<ANeonPlayerController>(GetController());
    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!PC || !EIC) return;

    EIC->BindAction(PC->IA_MoveForward, ETriggerEvent::Triggered, this, &APlayerShip::OnMoveForward);
    EIC->BindAction(PC->IA_MoveForward, ETriggerEvent::Completed, this, &APlayerShip::OnMoveForward);
    EIC->BindAction(PC->IA_MoveRight,   ETriggerEvent::Triggered, this, &APlayerShip::OnMoveRight);
    EIC->BindAction(PC->IA_MoveRight,   ETriggerEvent::Completed, this, &APlayerShip::OnMoveRight);
    EIC->BindAction(PC->IA_Thrust,      ETriggerEvent::Triggered, this, &APlayerShip::OnThrust);
    EIC->BindAction(PC->IA_Thrust,      ETriggerEvent::Completed, this, &APlayerShip::OnThrust);
    EIC->BindAction(PC->IA_Look,        ETriggerEvent::Triggered, this, &APlayerShip::OnLook);
    EIC->BindAction(PC->IA_Fire,        ETriggerEvent::Started,   this, &APlayerShip::StartFire);
    EIC->BindAction(PC->IA_Fire,        ETriggerEvent::Completed, this, &APlayerShip::StopFire);
    EIC->BindAction(PC->IA_Ready,       ETriggerEvent::Started,   this, &APlayerShip::OnReady);
}

void APlayerShip::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // When boarded in turret: freeze ship, route fire to turret
    ANeonPlayerController* PC = Cast<ANeonPlayerController>(GetController());
    if (PC && PC->BoardedTurret)
    {
        Velocity = FVector::ZeroVector;
        if (bFiring)
        {
            FireCooldown -= DeltaTime;
            if (FireCooldown <= 0.f)
            {
                PC->BoardedTurret->Fire();
                FireCooldown = 1.f / FMath::Max(0.1f, Stats.FireRate);
            }
        }
        return;
    }

    // --- Flight physics (hover-craft: no gravity, uniform drag on all axes) ---
    FRotator ControlRot = GetControlRotation();
    FVector  FwdXY      = FRotator(0, ControlRot.Yaw, 0).Vector();
    FVector  RightXY    = FVector::CrossProduct(FVector::UpVector, FwdXY);

    FVector InputDir = FwdXY * MoveForwardInput + RightXY * MoveRightInput;
    if (!InputDir.IsNearlyZero()) InputDir.Normalize();

    FVector Accel = (InputDir + FVector::UpVector * ThrustInput) * Stats.Acceleration;
    Velocity += Accel * DeltaTime;

    // Uniform drag on all axes — releasing input slowly coasts to stop
    float DragFactor = FMath::Exp(-Stats.LinearDrag * DeltaTime);
    Velocity *= DragFactor;

    // Auto-hover: when no vertical input, quickly damp Z to zero (hover in place)
    if (FMath::Abs(ThrustInput) < 0.05f)
        Velocity.Z = FMath::FInterpTo(Velocity.Z, 0.f, DeltaTime, 6.f);

    // Speed clamp (all directions)
    float Speed = Velocity.Size();
    if (Speed > Stats.MaxSpeed) Velocity = Velocity.GetSafeNormal() * Stats.MaxSpeed;

    AddActorWorldOffset(Velocity * DeltaTime, true);

    // Ship yaw follows controller (camera direction), roll leans on strafe for visual flair
    float TargetRoll  = MoveRightInput * 18.f;
    FRotator TargetRot(0.f, ControlRot.Yaw, TargetRoll);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 8.f));

    // Dynamic FOV
    float SpeedFrac = FMath::Clamp(Speed / FMath::Max(1.f, Stats.MaxSpeed), 0.f, 1.f);
    Camera->FieldOfView = FMath::Lerp(70.f, 110.f, SpeedFrac);

    // Aim muzzle along control rotation
    Muzzle->SetWorldRotation(ControlRot);

    // Fire
    if (bFiring)
    {
        FireCooldown -= DeltaTime;
        if (FireCooldown <= 0.f)
        {
            FireOnce();
            FireCooldown = 1.f / FMath::Max(0.1f, Stats.FireRate);
        }
    }

    CollectShards();
}

void APlayerShip::OnMoveForward(const FInputActionValue& Value) { MoveForwardInput = Value.Get<float>(); }
void APlayerShip::OnMoveRight  (const FInputActionValue& Value) { MoveRightInput   = Value.Get<float>(); }
void APlayerShip::OnThrust     (const FInputActionValue& Value)
{
    ThrustInput = FMath::Clamp(Value.Get<float>(), -1.f, 1.f);
}

void APlayerShip::OnLook(const FInputActionValue& Value)
{
    FVector2D Delta = Value.Get<FVector2D>();
    AddControllerYawInput  ( Delta.X * 0.15f);
    AddControllerPitchInput(-Delta.Y * 0.15f);
}

void APlayerShip::StartFire(const FInputActionValue&) { bFiring = true;  FireCooldown = 0.f; }
void APlayerShip::StopFire (const FInputActionValue&) { bFiring = false; }

void APlayerShip::OnReady(const FInputActionValue&)
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM) GM->RequestStartWave();
}

void APlayerShip::FireOnce()
{
    // If boarded in turret, fire the turret instead
    ANeonPlayerController* PC = Cast<ANeonPlayerController>(GetController());
    if (PC && PC->BoardedTurret)
    {
        PC->BoardedTurret->Fire();
        return;
    }

    FVector Start = Muzzle->GetComponentLocation();
    FVector End   = Start + Muzzle->GetForwardVector() * 8000.f;

    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 0.05f, 0, 2.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (IDamageable* D = Cast<IDamageable>(Hit.GetActor()))
            D->TakeHit(Stats.AttackDamage, Stats.AttackPower);
    }
}

void APlayerShip::CollectShards()
{
    FVector MyLoc = GetActorLocation();
    float   RadSq = Stats.MagnetRadius * Stats.MagnetRadius;

    for (TActorIterator<AResourceShard> It(GetWorld()); It; ++It)
    {
        if (FVector::DistSquared(MyLoc, It->GetActorLocation()) <= RadSq)
            It->AttractTo(this);
    }
}

void APlayerShip::TakeHit(float Damage, int32)
{
    CurrentHP -= Damage;
    if (CurrentHP <= 0.f)
    {
        ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
        if (GM) GM->OnBaseDestroyed();
    }
}
