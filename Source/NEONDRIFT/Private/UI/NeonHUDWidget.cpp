#include "NeonHUDWidget.h"
#include "NeonGameMode.h"
#include "NeonGameInstance.h"
#include "NeonPlayerController.h"
#include "NeonBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"

namespace HC
{
    const FLinearColor White  (1.f, 1.f, 1.f, 1.f);
    const FLinearColor Cyan   (0.f, 1.f, 1.f, 1.f);
    const FLinearColor Orange (1.f, 0.5f, 0.f, 1.f);
    const FLinearColor Red    (1.f, 0.2f, 0.2f, 1.f);
    const FLinearColor Green  (0.2f, 1.f, 0.2f, 1.f);
    const FLinearColor Gray   (0.5f, 0.5f, 0.5f, 1.f);
    const FLinearColor HPBg   (0.5f, 0.5f, 0.5f, 0.5f);
    const FLinearColor ShopBg (0.f, 0.02f, 0.05f, 0.88f);
}

UTextBlock* UNeonHUDWidget::MakeText(FLinearColor Col, float Scale)
{
    UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    T->SetColorAndOpacity(FSlateColor(Col));
    FSlateFontInfo Font = T->GetFont();
    Font.Size = 14.f * Scale;
    T->SetFont(Font);
    return T;
}

UImage* UNeonHUDWidget::MakeSolidRect(FLinearColor Col)
{
    UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
    Brush.OutlineSettings.CornerRadii = FVector4(0.f, 0.f, 0.f, 0.f);
    Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    Brush.TintColor = FSlateColor(FLinearColor::White);
    Img->SetBrush(Brush);
    Img->SetColorAndOpacity(Col);
    return Img;
}

UCanvasPanelSlot* UNeonHUDWidget::PlaceAt(UCanvasPanel* Canvas, UWidget* W,
    FVector2D Pos, FVector2D Size, FVector2D Anchor, FVector2D Align)
{
    UCanvasPanelSlot* CS = Canvas->AddChildToCanvas(W);
    CS->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
    CS->SetAlignment(Align);
    CS->SetPosition(Pos);
    CS->SetSize(Size);
    return CS;
}

void UNeonHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildLayout();
}

void UNeonHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UNeonHUDWidget::BuildLayout()
{
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    WidgetTree->RootWidget = Root;

    // ── Resources (top-left) ─────────────────────────────
    TxtResources = MakeText(HC::Orange);
    PlaceAt(Root, TxtResources, {20.f, 20.f}, {260.f, 28.f});

    // ── Wave label (top-center) ───────────────────────────
    TxtWave = MakeText(HC::Cyan, 1.2f);
    PlaceAt(Root, TxtWave, {0.f, 20.f}, {220.f, 30.f}, {0.5f, 0.f}, {0.5f, 0.f});

    // ── Base HP bar (top-right) ───────────────────────────
    ImgHPBg   = MakeSolidRect(HC::HPBg);
    ImgHPFill = MakeSolidRect(HC::Green);
    PlaceAt(Root, ImgHPBg,   {-20.f,  20.f}, {200.f, 20.f}, {1.f, 0.f}, {1.f, 0.f});
    SlotHPFill = PlaceAt(Root, ImgHPFill, {-220.f, 20.f}, {200.f, 20.f}, {1.f, 0.f}, {0.f, 0.f});

    TxtHP = MakeText(HC::White, 0.85f);
    PlaceAt(Root, TxtHP, {-20.f, 44.f}, {220.f, 22.f}, {1.f, 0.f}, {1.f, 0.f});

    // ── Phase overlay (center) ────────────────────────────
    TxtPhase = MakeText(HC::Cyan, 1.1f);
    TxtPhase->SetVisibility(ESlateVisibility::Collapsed);
    PlaceAt(Root, TxtPhase, {0.f, -14.f}, {520.f, 28.f}, {0.5f, 0.5f}, {0.5f, 0.5f});

    // ── Combat text (top, below wave) ─────────────────────
    TxtCombat = MakeText(HC::Red);
    TxtCombat->SetVisibility(ESlateVisibility::Collapsed);
    PlaceAt(Root, TxtCombat, {0.f, 60.f}, {280.f, 28.f}, {0.5f, 0.f}, {0.5f, 0.f});

    // ── GameOver / Victory ────────────────────────────────
    TxtGameState = MakeText(HC::Red, 2.5f);
    TxtGameState->SetVisibility(ESlateVisibility::Collapsed);
    PlaceAt(Root, TxtGameState, {0.f, -30.f}, {360.f, 70.f}, {0.5f, 0.5f}, {0.5f, 0.5f});

    TxtRestart = MakeText(HC::White);
    TxtRestart->SetText(FText::FromString(TEXT("[R] Restart")));
    TxtRestart->SetVisibility(ESlateVisibility::Collapsed);
    PlaceAt(Root, TxtRestart, {0.f, 40.f}, {200.f, 28.f}, {0.5f, 0.5f}, {0.5f, 0.5f});

    // ── Shop Panel (720x480, dark bg, cyan border) ────────
    PanelShop = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    PlaceAt(Root, PanelShop, {0.f, 0.f}, {720.f, 480.f}, {0.5f, 0.5f}, {0.5f, 0.5f});
    PanelShop->SetVisibility(ESlateVisibility::Collapsed);

    // Background (stretch to fill panel)
    UImage* ShopBg = MakeSolidRect(HC::ShopBg);
    UCanvasPanelSlot* BgSlot = PanelShop->AddChildToCanvas(ShopBg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Border lines (top, bottom, left, right)
    auto AddBorder = [&](FVector2D Pos, FVector2D Size) {
        PlaceAt(PanelShop, MakeSolidRect(HC::Cyan), Pos, Size);
    };
    AddBorder({0.f,   0.f  }, {720.f, 2.f  });
    AddBorder({0.f,   478.f}, {720.f, 2.f  });
    AddBorder({0.f,   0.f  }, {2.f,   480.f});
    AddBorder({718.f, 0.f  }, {2.f,   480.f});

    // Title
    UTextBlock* ShopTitle = MakeText(HC::Cyan, 1.8f);
    ShopTitle->SetText(FText::FromString(TEXT("UPGRADE SHOP")));
    PlaceAt(PanelShop, ShopTitle, {210.f, 18.f}, {300.f, 50.f});

    // Resources text
    TxtShopRes = MakeText(HC::Orange, 1.3f);
    PlaceAt(PanelShop, TxtShopRes, {255.f, 55.f}, {210.f, 35.f});

    // Divider below title area
    PlaceAt(PanelShop, MakeSolidRect(HC::Cyan), {20.f, 85.f}, {680.f, 1.f});

    // Column headers
    struct { const TCHAR* Lbl; float X; float W; } Headers[] = {
        {TEXT("Upgrade"), 30.f,  200.f},
        {TEXT("Effect"),  320.f, 200.f},
        {TEXT("Lv"),      530.f, 60.f },
        {TEXT("Cost"),    620.f, 80.f },
    };
    for (const auto& H : Headers)
    {
        UTextBlock* T = MakeText(HC::Gray, 0.95f);
        T->SetText(FText::FromString(H.Lbl));
        PlaceAt(PanelShop, T, {H.X, 90.f}, {H.W, 25.f});
    }

    // Upgrade rows
    const float RowH = 38.f, StartY = 115.f;
    for (int32 i = 0; i < MaxShopRows; ++i)
    {
        FShopRow R;
        R.Name   = MakeText(HC::White,  1.1f);
        R.Effect = MakeText(HC::Gray,   1.0f);
        R.Lvl    = MakeText(HC::White,  1.0f);
        R.Cost   = MakeText(HC::Orange, 1.1f);
        const float Y = StartY + i * RowH;
        PlaceAt(PanelShop, R.Name,   {30.f,  Y + 8.f}, {280.f, 25.f});
        PlaceAt(PanelShop, R.Effect, {320.f, Y + 8.f}, {200.f, 25.f});
        PlaceAt(PanelShop, R.Lvl,   {530.f, Y + 8.f}, {60.f,  25.f});
        PlaceAt(PanelShop, R.Cost,  {620.f, Y + 8.f}, {80.f,  25.f});
        UTextBlock* All[] = {R.Name, R.Effect, R.Lvl, R.Cost};
        for (UTextBlock* T : All) T->SetVisibility(ESlateVisibility::Collapsed);
        ShopRows.Add(R);
    }

    // Footer divider + controls
    PlaceAt(PanelShop, MakeSolidRect(HC::Cyan), {20.f, 420.f}, {680.f, 1.f});
    UTextBlock* Ctrl = MakeText(HC::White);
    Ctrl->SetText(FText::FromString(TEXT("[UP/DOWN] Navigate    [ENTER] Buy    [TAB] Next Wave")));
    PlaceAt(PanelShop, Ctrl, {30.f, 428.f}, {660.f, 28.f});
}

void UNeonHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ANeonGameMode* GM = Cast<ANeonGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    // Resources
    TxtResources->SetText(FText::FromString(
        FString::Printf(TEXT("Resources: %d"), GM->Resources)));

    // Wave
    TxtWave->SetText(FText::FromString(
        FString::Printf(TEXT("Wave %d / %d"), GM->WaveIndex + 1, GM->WaveTable.Num())));

    // HP bar
    if (GM->Base)
    {
        const float Frac = FMath::Clamp(GM->Base->GetHPFraction(), 0.f, 1.f);
        ImgHPFill->SetColorAndOpacity(Frac > 0.25f ? HC::Green : HC::Red);
        SlotHPFill->SetSize(FVector2D(200.f * Frac, 20.f));
        TxtHP->SetText(FText::FromString(
            FString::Printf(TEXT("Base HP: %.0f / %.0f"), GM->Base->CurrentHP, GM->Base->MaxHP)));
    }

    // Phase visibility flags
    bool bCenter = false, bCombat = false, bEndState = false;
    switch (GM->Phase)
    {
    case EGamePhase::PreWave:
        TxtPhase->SetText(FText::FromString(
            FString::Printf(TEXT("Wave %d — Press [F] to start gathering"), GM->WaveIndex + 1)));
        TxtPhase->SetColorAndOpacity(FSlateColor(HC::Cyan));
        bCenter = true;
        break;

    case EGamePhase::Gather:
        TxtPhase->SetText(FText::FromString(
            FString::Printf(TEXT("Gather Resources! [F] Ready    %.0fs left"), GM->GatherTimer)));
        TxtPhase->SetColorAndOpacity(FSlateColor(HC::Orange));
        bCenter = true;
        break;

    case EGamePhase::Combat:
    {
        const int32 Left = GM->TotalMonstersThisWave - GM->SpawnedThisWave + GM->AliveMonsters;
        TxtCombat->SetText(FText::FromString(
            FString::Printf(TEXT("Enemies remaining: %d"), FMath::Max(0, Left))));
        bCombat = true;
        break;
    }
    case EGamePhase::GameOver:
        TxtGameState->SetText(FText::FromString(TEXT("GAME OVER")));
        TxtGameState->SetColorAndOpacity(FSlateColor(HC::Red));
        bEndState = true;
        break;

    case EGamePhase::Victory:
        TxtGameState->SetText(FText::FromString(TEXT("VICTORY!")));
        TxtGameState->SetColorAndOpacity(FSlateColor(HC::Cyan));
        bEndState = true;
        break;

    default: break;
    }

    auto Vis = [](bool b) { return b ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed; };
    TxtPhase->SetVisibility(Vis(bCenter));
    TxtCombat->SetVisibility(Vis(bCombat));
    TxtGameState->SetVisibility(Vis(bEndState));
    TxtRestart->SetVisibility(Vis(bEndState));

    // Shop panel
    const bool bShop = (GM->Phase == EGamePhase::Shop);
    PanelShop->SetVisibility(Vis(bShop));

    if (bShop)
    {
        TxtShopRes->SetText(FText::FromString(
            FString::Printf(TEXT("Resources: %d"), GM->Resources)));

        ANeonPlayerController* PC = Cast<ANeonPlayerController>(GetOwningPlayer());
        UNeonGameInstance*     GI = Cast<UNeonGameInstance>(GetGameInstance());
        const int32 ShopCursor = PC ? PC->ShopCursorIndex : 0;

        for (int32 i = 0; i < MaxShopRows; ++i)
        {
            FShopRow& R = ShopRows[i];
            if (i >= GM->UpgradeTable.Num())
            {
                UTextBlock* All[] = {R.Name, R.Effect, R.Lvl, R.Cost};
                for (UTextBlock* T : All) T->SetVisibility(ESlateVisibility::Collapsed);
                continue;
            }
            UTextBlock* All[] = {R.Name, R.Effect, R.Lvl, R.Cost};
            for (UTextBlock* T : All) T->SetVisibility(ESlateVisibility::HitTestInvisible);

            const FUpgradeDef& U     = GM->UpgradeTable[i];
            const int32        CurLvl = GI ? GI->Upgrades.GetLevel(U.Id) : 0;
            const bool bMaxed    = (CurLvl >= U.MaxLevel);
            const bool bSelected = (i == ShopCursor);
            const bool bAfford   = (GM->Resources >= U.Cost);

            const FLinearColor NameCol = bMaxed ? HC::Gray  : (bSelected ? HC::Cyan   : HC::White);
            const FLinearColor CostCol = bMaxed ? HC::Gray  : (bAfford   ? HC::Orange : HC::Red);

            R.Name->SetText(FText::FromString(U.DisplayName));
            R.Name->SetColorAndOpacity(FSlateColor(NameCol));

            R.Effect->SetText(FText::FromString(U.EffectDesc));

            R.Lvl->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurLvl, U.MaxLevel)));
            R.Lvl->SetColorAndOpacity(FSlateColor(NameCol));

            R.Cost->SetText(FText::FromString(bMaxed ? TEXT("MAX") : FString::Printf(TEXT("%d"), U.Cost)));
            R.Cost->SetColorAndOpacity(FSlateColor(CostCol));
        }
    }
}