#include "NeonHUD.h"
#include "NeonHUDWidget.h"
#include "NeonPlayerController.h"
#include "ManualTurret.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"

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

void ANeonHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    // Turret dual-crosshair — kept on Canvas because barrel dot needs world→screen projection
    ANeonPlayerController* PC = Cast<ANeonPlayerController>(GetOwningPlayerController());
    if (!PC || !PC->BoardedTurret) return;

    const float CX = Canvas->SizeX * 0.5f, CY = Canvas->SizeY * 0.5f;
    constexpr float Len = 16.f, Gap = 4.f;

    DrawLine(CX - Len, CY, CX - Gap, CY, FLinearColor::White, 1.5f);
    DrawLine(CX + Gap, CY, CX + Len, CY, FLinearColor::White, 1.5f);
    DrawLine(CX, CY - Len, CX, CY - Gap, FLinearColor::White, 1.5f);
    DrawLine(CX, CY + Gap, CX, CY + Len, FLinearColor::White, 1.5f);

    FVector BarrelTip = PC->PlayerCameraManager->GetCameraLocation()
                      + PC->BoardedTurret->GetBarrelForward() * 8000.f;
    FVector2D DotScreen;
    if (PC->ProjectWorldLocationToScreen(BarrelTip, DotScreen))
    {
        constexpr float S = 5.f;
        DrawRect(FLinearColor(1.f, 0.7f, 0.f, 1.f), DotScreen.X - S, DotScreen.Y - S, S * 2.f, S * 2.f);
    }
}
