#include "PlayerShip.h"
#include "NeonGameMode.h"
#include "NeonGameInstance.h"
#include "ResourceShard.h"
#include "NeonPlayerController.h"
#include "ManualTurret.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/AudioComponent.h"
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

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NeonMat(TEXT("/Game/Materials/M_NeonBase.M_NeonBase"));
    if (NeonMat.Succeeded())
        Mesh->SetMaterial(0, NeonMat.Object);

    TrailISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Trail"));
    TrailISMC->SetupAttachment(RootComponent);
    TrailISMC->SetCollisionProfileName(TEXT("NoCollision"));
    TrailISMC->SetCastShadow(false);
    if (CubeMesh.Succeeded())  TrailISMC->SetStaticMesh(CubeMesh.Object);
    if (NeonMat.Succeeded())   TrailISMC->SetMaterial(0, NeonMat.Object);

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

    EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
    EngineAudio->SetupAttachment(RootComponent);
    EngineAudio->bAutoActivate = false;
}

void APlayerShip::BeginPlay()
{
    Super::BeginPlay();
    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    if (GI) ApplyStats(GI->GetShipStats());
    CurrentHP = Stats.MaxHP;

    ShipMID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
    if (ShipMID)
    {
        ShipMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 1.f, 1.f)); // Cyan
        ShipMID->SetScalarParameterValue(TEXT("Glow"), 3.f);
    }

    if (UMaterialInstanceDynamic* TrailMID = TrailISMC->CreateAndSetMaterialInstanceDynamic(0))
    {
        TrailMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 1.f, 1.f));
        TrailMID->SetScalarParameterValue(TEXT("Glow"), 1.5f);
    }

    if (EngineSound)
    {
        EngineAudio->SetSound(EngineSound);
        EngineAudio->Play();
    }
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
    EIC->BindAction(PC->IA_ThrustUp,    ETriggerEvent::Triggered, this, &APlayerShip::OnThrustUp);
    EIC->BindAction(PC->IA_ThrustUp,    ETriggerEvent::Completed, this, &APlayerShip::OnThrustUp);
    EIC->BindAction(PC->IA_ThrustDown,  ETriggerEvent::Triggered, this, &APlayerShip::OnThrustDown);
    EIC->BindAction(PC->IA_ThrustDown,  ETriggerEvent::Completed, this, &APlayerShip::OnThrustDown);
    EIC->BindAction(PC->IA_Look,        ETriggerEvent::Triggered, this, &APlayerShip::OnLook);
    EIC->BindAction(PC->IA_Fire,        ETriggerEvent::Started,   this, &APlayerShip::StartFire);
    EIC->BindAction(PC->IA_Fire,        ETriggerEvent::Completed, this, &APlayerShip::StopFire);
    EIC->BindAction(PC->IA_Ready,       ETriggerEvent::Started,   this, &APlayerShip::OnReady);
}

void APlayerShip::UpdateTrail(float DeltaTime)
{
    // Age + cull
    for (int32 i = TrailAge.Num() - 1; i >= 0; i--)
    {
        TrailAge[i] += DeltaTime;
        if (TrailAge[i] >= 0.4f)
        {
            TrailPos.RemoveAt(i);
            TrailRot.RemoveAt(i);
            TrailAge.RemoveAt(i);
        }
    }
    // Sample
    TrailSampleTimer += DeltaTime;
    if (TrailSampleTimer >= 0.08f)
    {
        TrailSampleTimer = 0.f;
        TrailPos.Insert(GetActorLocation(), 0);
        TrailRot.Insert(GetActorQuat(),     0);
        TrailAge.Insert(0.f,               0);
        if (TrailPos.Num() > 10) { TrailPos.SetNum(10); TrailRot.SetNum(10); TrailAge.SetNum(10); }
    }
    // Rebuild ISMC
    if (!TrailISMC) return;
    TrailISMC->ClearInstances();
    for (int32 i = 0; i < TrailPos.Num(); i++)
    {
        float Alpha = 1.f - TrailAge[i] / 0.4f;
        FVector Scale = FVector(2.f, 1.f, 0.4f) * Alpha * 0.8f;
        TrailISMC->AddInstance(FTransform(TrailRot[i], TrailPos[i], Scale), true);
    }
}

void APlayerShip::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateTrail(DeltaTime);

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
    FVector  Fwd        = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
    FVector  Right      = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);

    FVector InputDir = Fwd * MoveForwardInput + Right * MoveRightInput;
    if (!InputDir.IsNearlyZero()) InputDir.Normalize();
    InputDir.Z *= 2.f;

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

    // Ship follows camera yaw, tilts on strafe (roll) and look pitch (clamped)
    float TargetRoll  = MoveRightInput * 18.f;
    float TargetPitch = FMath::Clamp(FRotator::NormalizeAxis(ControlRot.Pitch), -25.f, 25.f);
    FRotator TargetRot(TargetPitch, ControlRot.Yaw, TargetRoll);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 8.f));

    // Dynamic FOV + engine audio pitch
    float SpeedFrac = FMath::Clamp(Speed / FMath::Max(1.f, Stats.MaxSpeed), 0.f, 1.f);
    Camera->FieldOfView = FMath::Lerp(70.f, 110.f, SpeedFrac);
    if (EngineAudio && EngineAudio->IsPlaying())
    {
        EngineAudio->SetPitchMultiplier (FMath::Lerp(0.6f, 1.4f, SpeedFrac));
        EngineAudio->SetVolumeMultiplier(FMath::Lerp(0.3f, 1.0f, SpeedFrac));
    }

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
void APlayerShip::OnThrustUp(const FInputActionValue& Value)
{
    bThrustUp   = Value.Get<bool>();
    ThrustInput = bThrustUp ? 1.f : (bThrustDown ? -1.f : 0.f);
}
void APlayerShip::OnThrustDown(const FInputActionValue& Value)
{
    bThrustDown = Value.Get<bool>();
    ThrustInput = bThrustDown ? -1.f : (bThrustUp ? 1.f : 0.f);
}

void APlayerShip::OnLook(const FInputActionValue& Value)
{
    FVector2D Delta = Value.Get<FVector2D>();
    AddControllerYawInput  ( Delta.X * 0.09f);
    AddControllerPitchInput(-Delta.Y * 0.09f);
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
    if (FireSound) UGameplayStatics::SpawnSoundAtLocation(this, FireSound, Start);

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
    if (HitSound) UGameplayStatics::SpawnSoundAtLocation(this, HitSound, GetActorLocation());
    if (CurrentHP <= 0.f)
    {
        ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
        if (GM) GM->OnBaseDestroyed();
    }
}
