#include "NeonPlayerController.h"
#include "NeonGameMode.h"
#include "NeonGameInstance.h"
#include "ManualTurret.h"
#include "PlayerShip.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ANeonPlayerController::ANeonPlayerController()
{
    bShowMouseCursor   = false;
    DefaultMouseCursor = EMouseCursor::Crosshairs;
    PrimaryActorTick.bCanEverTick = true;
}

// Helper: create a 1D Axis action and add W/positive + S/negative key mappings
static UInputAction* Make1DAction(UObject* Outer, UInputMappingContext* IMC,
                                    FKey PosKey, FKey NegKey,
                                    const TCHAR* Name)
{
    UInputAction* IA = NewObject<UInputAction>(Outer, Name);
    IA->ValueType = EInputActionValueType::Axis1D;

    IMC->MapKey(IA, PosKey); // positive: no modifier

    FEnhancedActionKeyMapping& NegMap = IMC->MapKey(IA, NegKey);
    UInputModifierNegate* Neg = NewObject<UInputModifierNegate>(Outer);
    NegMap.Modifiers.Add(Neg);

    return IA;
}

static UInputAction* MakeBoolAction(UObject* Outer, UInputMappingContext* IMC,
                                        FKey Key, const TCHAR* Name)
{
    UInputAction* IA = NewObject<UInputAction>(Outer, Name);
    IA->ValueType = EInputActionValueType::Boolean;
    IMC->MapKey(IA, Key);
    return IA;
}

void ANeonPlayerController::BuildIMC()
{
    IMC = NewObject<UInputMappingContext>(this, TEXT("IMC_Neon"));

    IA_MoveForward = Make1DAction(this, IMC, EKeys::W, EKeys::S, TEXT("IA_MoveForward"));
    IA_MoveRight   = Make1DAction(this, IMC, EKeys::D, EKeys::A, TEXT("IA_MoveRight"));
    IA_Thrust      = Make1DAction(this, IMC, EKeys::SpaceBar, EKeys::LeftShift, TEXT("IA_Thrust"));

    // Look (Mouse2D → Axis2D)
    IA_Look = NewObject<UInputAction>(this, TEXT("IA_Look"));
    IA_Look->ValueType = EInputActionValueType::Axis2D;
    IMC->MapKey(IA_Look, EKeys::Mouse2D);

    IA_Fire        = MakeBoolAction(this, IMC, EKeys::LeftMouseButton, TEXT("IA_Fire"));
    IA_Ready       = MakeBoolAction(this, IMC, EKeys::F,               TEXT("IA_Ready"));
    IA_ShopUp      = MakeBoolAction(this, IMC, EKeys::Up,              TEXT("IA_ShopUp"));
    IA_ShopDown    = MakeBoolAction(this, IMC, EKeys::Down,            TEXT("IA_ShopDown"));
    IA_ShopConfirm = MakeBoolAction(this, IMC, EKeys::Enter,           TEXT("IA_ShopConfirm"));
    IA_NextWave    = MakeBoolAction(this, IMC, EKeys::Tab,             TEXT("IA_NextWave"));
    IA_Restart     = MakeBoolAction(this, IMC, EKeys::R,               TEXT("IA_Restart"));
    IA_Interact    = MakeBoolAction(this, IMC, EKeys::E,               TEXT("IA_Interact"));

    if (UEnhancedInputLocalPlayerSubsystem* Sub =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Sub->AddMappingContext(IMC, 0);
    }
}

void ANeonPlayerController::BeginPlay()
{
    Super::BeginPlay();
    // Limit vertical look range — prevents disorienting straight-up/down views
    if (PlayerCameraManager)
    {
        PlayerCameraManager->ViewPitchMin = -60.f;
        PlayerCameraManager->ViewPitchMax = 60.f;
    }
}

void ANeonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent(); // Creates InputComponent

    BuildIMC();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EIC->BindAction(IA_ShopUp,      ETriggerEvent::Started, this, &ANeonPlayerController::OnShopUp);
        EIC->BindAction(IA_ShopDown,    ETriggerEvent::Started, this, &ANeonPlayerController::OnShopDown);
        EIC->BindAction(IA_ShopConfirm, ETriggerEvent::Started, this, &ANeonPlayerController::OnShopConfirm);
        EIC->BindAction(IA_NextWave,    ETriggerEvent::Started, this, &ANeonPlayerController::OnNextWave);
        EIC->BindAction(IA_Restart,     ETriggerEvent::Started, this, &ANeonPlayerController::OnRestart);
        EIC->BindAction(IA_Interact,    ETriggerEvent::Started, this, &ANeonPlayerController::OnInteract);
    }
}

void ANeonPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Forward control rotation to manual turret each frame
    for (TActorIterator<AManualTurret> It(GetWorld()); It; ++It)
    {
        It->DesiredAim = GetControlRotation();
        break;
    }
}

void ANeonPlayerController::OnShopUp()
{
    ShopCursorIndex = FMath::Max(0, ShopCursorIndex - 1);
    if (ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this)))
        if (GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
}

void ANeonPlayerController::OnShopDown()
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    int32 Max = GM ? GM->UpgradeTable.Num() - 1 : 0;
    ShopCursorIndex = FMath::Min(Max, ShopCursorIndex + 1);
    if (GM && GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
}

void ANeonPlayerController::OnShopConfirm()
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM || GM->Phase != EGamePhase::Shop) return;
    if (!GM->UpgradeTable.IsValidIndex(ShopCursorIndex)) return;
    GM->ApplyUpgrade(GM->UpgradeTable[ShopCursorIndex].Id);
}

void ANeonPlayerController::OnNextWave()
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM) GM->ContinueToNextWave();
}

void ANeonPlayerController::OnInteract()
{
    if (BoardedTurret)
    {
        BoardedTurret->SetPlayerBoarded(false);
        BoardedTurret = nullptr;
        SetViewTargetWithBlend(GetPawn(), 0.25f);
        return;
    }

    APlayerShip* Ship = Cast<APlayerShip>(GetPawn());
    if (!Ship) return;

    FVector ShipLoc = Ship->GetActorLocation();
    for (TActorIterator<AManualTurret> It(GetWorld()); It; ++It)
    {
        if (FVector::Dist(ShipLoc, It->GetActorLocation()) <= It->BoardingRadius)
        {
            BoardedTurret = *It;
            BoardedTurret->SetPlayerBoarded(true);
            SetViewTargetWithBlend(BoardedTurret, 0.25f);
            return;
        }
    }
}

void ANeonPlayerController::OnRestart()
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM || (GM->Phase != EGamePhase::GameOver && GM->Phase != EGamePhase::Victory)) return;

    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    if (GI) GI->ResetRun();

    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
