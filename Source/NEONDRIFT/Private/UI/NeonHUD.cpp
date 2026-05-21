#include "NeonHUD.h"
#include "NeonHUDWidget.h"
#include "NeonPlayerController.h"
#include "ManualTurret.h"
#include "Monster.h"
#include "ResourceBlock.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"

void ANeonHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    HUDWidget = CreateWidget<UNeonHUDWidget>(PC, UNeonHUDWidget::StaticClass());
    if (!HUDWidget) return;

    HUDWidget->AddToViewport();

    // Prevent UMG from capturing keyboard input (Enter, arrows, etc.)
    PC->SetInputMode(FInputModeGameOnly());
}

static void DrawWorldBar(AHUD* HUD, APlayerController* PC,
                            FVector WorldLoc, float Fraction, FLinearColor Color, float ZOffset = 120.f)
{
    FVector2D Screen;
    if (!PC->ProjectWorldLocationToScreen(WorldLoc + FVector(0.f, 0.f, ZOffset), Screen, true))
        return;

    constexpr float W = 60.f, H = 7.f;
    const float X = Screen.X - W * 0.5f, Y = Screen.Y;

    HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.75f), X, Y, W, H);
    if (Fraction > 0.f)
        HUD->DrawRect(Color, X, Y, W * FMath::Clamp(Fraction, 0.f, 1.f), H);
}

void ANeonHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    // --- World-space HP bars ---
    for (TActorIterator<AMonster> It(GetWorld()); It; ++It)
        DrawWorldBar(this, PC, It->GetActorLocation(),
                    It->MaxHP > 0.f ? It->HP / It->MaxHP : 0.f,
                    FLinearColor(1.f, 0.1f, 0.4f));

    for (TActorIterator<AResourceBlock> It(GetWorld()); It; ++It)
        DrawWorldBar(this, PC, It->GetActorLocation(),
                    It->GetHPFraction(), It->GetBarColor(), 80.f);

    // --- Turret dual-crosshair ---
    ANeonPlayerController* NeonPC = Cast<ANeonPlayerController>(PC);
    if (!NeonPC || !NeonPC->BoardedTurret) return;

    const float CX = Canvas->SizeX * 0.5f, CY = Canvas->SizeY * 0.5f;
    constexpr float Len = 16.f, Gap = 4.f;

    DrawLine(CX - Len, CY, CX - Gap, CY, FLinearColor::White, 1.5f);
    DrawLine(CX + Gap, CY, CX + Len, CY, FLinearColor::White, 1.5f);
    DrawLine(CX, CY - Len, CX, CY - Gap, FLinearColor::White, 1.5f);
    DrawLine(CX, CY + Gap, CX, CY + Len, FLinearColor::White, 1.5f);

    FVector BarrelAim = NeonPC->PlayerCameraManager->GetCameraLocation()
                        + NeonPC->BoardedTurret->GetBarrelForward() * 8000.f;
    FVector2D DotScreen;
    if (PC->ProjectWorldLocationToScreen(BarrelAim, DotScreen, true))
    {
        constexpr float S = 5.f;
        DrawRect(FLinearColor(1.f, 0.7f, 0.f, 1.f), DotScreen.X - S, DotScreen.Y - S, S * 2.f, S * 2.f);
    }
}
