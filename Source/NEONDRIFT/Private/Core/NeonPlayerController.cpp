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
#include "Kismet/KismetSystemLibrary.h"

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
    IA_ThrustUp   = MakeBoolAction(this, IMC, EKeys::SpaceBar,   TEXT("IA_ThrustUp"));
    IA_ThrustDown = MakeBoolAction(this, IMC, EKeys::C,          TEXT("IA_ThrustDown"));

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
    IA_Escape      = MakeBoolAction(this, IMC, EKeys::Escape,          TEXT("IA_Escape"));
    IMC->MapKey(IA_Escape, EKeys::I); // TODO: 패키징 전 제거 (에디터 테스트용)

    // 일시정지 중에도 메뉴 조작이 가능하도록 설정
    IA_Escape->bTriggerWhenPaused     = true;
    IA_ShopUp->bTriggerWhenPaused     = true;
    IA_ShopDown->bTriggerWhenPaused   = true;
    IA_ShopConfirm->bTriggerWhenPaused = true;

    if (UEnhancedInputLocalPlayerSubsystem* Sub =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Sub->AddMappingContext(IMC, 0);
    }
}

void ANeonPlayerController::BeginPlay()
{
    Super::BeginPlay();
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
        EIC->BindAction(IA_Escape,      ETriggerEvent::Started, this, &ANeonPlayerController::OnEscapePressed);
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
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    if (bPauseMenuOpen || GM->Phase == EGamePhase::MainMenu)
    {
        if (!bShowControlsPanel)
        {
            if (MenuCursorIndex > 0)
            {
                MenuCursorIndex--;
                if (GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
            }
            else
            {
                if (GM->ShopFailSound) UGameplayStatics::PlaySound2D(this, GM->ShopFailSound);
            }
        }
        return;
    }

    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    for (int32 i = ShopCursorIndex - 1; i >= 0; i--)
    {
        const FUpgradeDef& D = GM->UpgradeTable[i];
        if (!GI || GI->Upgrades.GetLevel(D.Id) < D.MaxLevel)
        {
            ShopCursorIndex = i;
            if (GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
            return;
        }
    }
}

void ANeonPlayerController::OnShopDown()
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    if (bPauseMenuOpen || GM->Phase == EGamePhase::MainMenu)
    {
        if (!bShowControlsPanel)
        {
            if (MenuCursorIndex < 2)
            {
                MenuCursorIndex++;
                if (GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
            }
            else
            {
                if (GM->ShopFailSound) UGameplayStatics::PlaySound2D(this, GM->ShopFailSound);
            }
        }
        return;
    }

    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());
    for (int32 i = ShopCursorIndex + 1; i < GM->UpgradeTable.Num(); i++)
    {
        const FUpgradeDef& D = GM->UpgradeTable[i];
        if (!GI || GI->Upgrades.GetLevel(D.Id) < D.MaxLevel)
        {
            ShopCursorIndex = i;
            if (GM->ShopNavigateSound) UGameplayStatics::PlaySound2D(this, GM->ShopNavigateSound);
            return;
        }
    }
}
void ANeonPlayerController::OnShopConfirm()
{
    if (bShowControlsPanel) { bShowControlsPanel = false; return; }

    if (bPauseMenuOpen)
    {
        ANeonGameMode* GMp = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
        if (GMp && GMp->ShopBuySound) UGameplayStatics::PlaySound2D(this, GMp->ShopBuySound);
        switch (MenuCursorIndex)
        {
        case 0: bPauseMenuOpen = false; MenuCursorIndex = 0; SetPause(false); break;
        case 1: bShowControlsPanel = true; break;
        case 2: UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false); break;
        }
        return;
    }

    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    if (GM->Phase == EGamePhase::MainMenu)
    {
        if (GM->ShopBuySound) UGameplayStatics::PlaySound2D(this, GM->ShopBuySound);
        switch (MenuCursorIndex)
        {
        case 0: MenuCursorIndex = 0; GM->StartGame(); break;
        case 1: bShowControlsPanel = true; break;
        case 2: UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false); break;
        }
        return;
    }

    if (GM->Phase != EGamePhase::Shop) return;
    if (!GM->UpgradeTable.IsValidIndex(ShopCursorIndex)) return;
    GM->ApplyUpgrade(GM->UpgradeTable[ShopCursorIndex].Id);
}

void ANeonPlayerController::OnEscapePressed()
{
    if (bShowControlsPanel) { bShowControlsPanel = false; return; }
    if (bPauseMenuOpen) { bPauseMenuOpen = false; MenuCursorIndex = 0; SetPause(false); return; }

    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    if (GM->Phase == EGamePhase::MainMenu)
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
        return;
    }
    if (GM->Phase == EGamePhase::GameOver || GM->Phase == EGamePhase::Victory) return;

    MenuCursorIndex = 0;
    bPauseMenuOpen  = true;
    SetPause(true);
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
