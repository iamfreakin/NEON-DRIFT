#include "NeonHUD.h"
#include "NeonGameMode.h"
#include "NeonGameInstance.h"
#include "NeonBase.h"
#include "NeonPlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

static const FLinearColor ColWhite  (1.f, 1.f, 1.f, 1.f);
static const FLinearColor ColCyan   (0.f, 1.f, 1.f, 1.f);
static const FLinearColor ColOrange (1.f, 0.5f, 0.f, 1.f);
static const FLinearColor ColRed    (1.f, 0.2f, 0.2f, 1.f);
static const FLinearColor ColGreen  (0.2f, 1.f, 0.2f, 1.f);
static const FLinearColor ColGray   (0.5f, 0.5f, 0.5f, 0.5f);

UFont* ANeonHUD::GetFont() const
{
    return GEngine ? GEngine->GetMediumFont() : nullptr;
}

void ANeonHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    float W = Canvas->SizeX;
    float H = Canvas->SizeY;

    DrawResources(20.f, 20.f);
    DrawBaseHP(W - 250.f, 20.f);

    // Wave label (top center)
    FString WaveStr = FString::Printf(TEXT("Wave %d / %d"), GM->WaveIndex + 1, GM->WaveTable.Num());
    DrawText(WaveStr, ColCyan, W * 0.5f - 60.f, 20.f, GetFont(), 1.2f, false);

    DrawPhaseInfo(W * 0.5f, H * 0.5f);

    if (GM->Phase == EGamePhase::Shop)
        DrawShopMenu(W * 0.5f, H * 0.4f);
}

void ANeonHUD::DrawResources(float X, float Y)
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    FString Str = FString::Printf(TEXT("Resources: %d"), GM->Resources);
    DrawText(Str, ColOrange, X, Y, GetFont(), 1.f, false);
}

void ANeonHUD::DrawBaseHP(float X, float Y)
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    ABase*         B  = GM ? GM->Base : nullptr;
    if (!B) return;

    float Frac = B->GetHPFraction();
    float BarW = 200.f, BarH = 20.f;

    // Background
    DrawRect(ColGray, X, Y, BarW, BarH);
    // Fill
    FLinearColor Fill = Frac > 0.25f ? ColGreen : ColRed;
    DrawRect(Fill, X, Y, BarW * Frac, BarH);
    // Label
    FString HPStr = FString::Printf(TEXT("Base HP: %.0f / %.0f"), B->CurrentHP, B->MaxHP);
    DrawText(HPStr, ColWhite, X, Y + BarH + 4.f, GetFont(), 0.85f, false);
}

void ANeonHUD::DrawPhaseInfo(float CX, float CY)
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    switch (GM->Phase)
    {
    case EGamePhase::PreWave:
    {
        FString S = FString::Printf(TEXT("Wave %d — Press [F] to start gathering"), GM->WaveIndex + 1);
        DrawText(S, ColCyan, CX - 200.f, CY - 14.f, GetFont(), 1.1f, false);
        break;
    }
    case EGamePhase::Gather:
    {
        FString S = FString::Printf(TEXT("Gather Resources! [F] Ready    %.0fs left"), GM->GatherTimer);
        DrawText(S, ColOrange, CX - 240.f, CY - 14.f, GetFont(), 1.f, false);
        break;
    }
    case EGamePhase::Combat:
    {
        int32 Remaining = GM->TotalMonstersThisWave - GM->SpawnedThisWave + GM->AliveMonsters;
        FString S = FString::Printf(TEXT("Enemies remaining: %d"), FMath::Max(0, Remaining));
        DrawText(S, ColRed, CX - 100.f, 60.f, GetFont(), 1.f, false);
        break;
    }
    case EGamePhase::GameOver:
        DrawText(TEXT("GAME OVER"), ColRed, CX - 100.f, CY - 30.f, GetFont(), 2.5f, false);
        DrawText(TEXT("[R] Restart"), ColWhite, CX - 60.f, CY + 40.f, GetFont(), 1.f, false);
        break;

    case EGamePhase::Victory:
        DrawText(TEXT("VICTORY!"), ColCyan, CX - 90.f, CY - 30.f, GetFont(), 2.5f, false);
        DrawText(TEXT("[R] Restart"), ColWhite, CX - 60.f, CY + 40.f, GetFont(), 1.f, false);
        break;

    default: break;
    }
}

void ANeonHUD::DrawShopMenu(float CX, float CY)
{
    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    ANeonPlayerController* PC = Cast<ANeonPlayerController>(GetOwningPlayerController());
    if (!GM || !PC) return;

    UNeonGameInstance* GI = Cast<UNeonGameInstance>(GetGameInstance());

    DrawText(TEXT("=== UPGRADE SHOP ==="), ColCyan, CX - 130.f, CY - 30.f, GetFont(), 1.2f, false);
    DrawText(FString::Printf(TEXT("Resources: %d"), GM->Resources), ColOrange, CX - 60.f, CY - 8.f, GetFont(), 1.f, false);

    float ItemY = CY + 20.f;
    float ItemH = 22.f;
    for (int32 i = 0; i < GM->UpgradeTable.Num(); i++)
    {
        const FUpgradeDef& U = GM->UpgradeTable[i];
        int32 CurLvl  = GI ? GI->Upgrades.GetLevel(U.Id) : 0;
        bool  MaxedOut = CurLvl >= U.MaxLevel;
        bool  Selected = (i == PC->ShopCursorIndex);
        bool  Afford   = (GM->Resources >= U.Cost);

        FLinearColor Col = MaxedOut ? ColGray : (Afford ? ColWhite : ColRed);
        if (Selected) Col = ColOrange;

        FString Line = FString::Printf(TEXT("%s  [%s]  Lv%d/%d  Cost:%d"),
            *U.DisplayName, *U.EffectDesc, CurLvl, U.MaxLevel, U.Cost);

        if (Selected) DrawRect(FLinearColor(0.2f, 0.2f, 0.f, 0.4f), CX - 230.f, ItemY - 2.f, 460.f, ItemH);
        DrawText(Line, Col, CX - 220.f, ItemY, GetFont(), 0.9f, false);
        ItemY += ItemH;
    }

    DrawText(TEXT("[Up/Down] Select   [Enter] Buy   [Tab] Next Wave"), ColGray, CX - 220.f, ItemY + 10.f, GetFont(), 0.85f, false);
}
