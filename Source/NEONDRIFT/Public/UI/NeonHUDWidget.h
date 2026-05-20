#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NeonHUDWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UTextBlock;
class UImage;
class UWidget;

UCLASS()
class NEONDRIFT_API UNeonHUDWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    void             BuildLayout();
    UTextBlock*      MakeText(FLinearColor Col, float Scale = 1.f);
    UImage*          MakeSolidRect(FLinearColor Col);
    UCanvasPanelSlot* PlaceAt(UCanvasPanel* Canvas, UWidget* W,
                              FVector2D Pos, FVector2D Size,
                              FVector2D Anchor = FVector2D::ZeroVector,
                              FVector2D Align  = FVector2D::ZeroVector);

    // Always-visible
    UTextBlock*       TxtResources = nullptr;
    UTextBlock*       TxtWave      = nullptr;
    UImage*           ImgHPBg      = nullptr;
    UImage*           ImgHPFill    = nullptr;
    UCanvasPanelSlot* SlotHPFill   = nullptr;
    UTextBlock*       TxtHP        = nullptr;

    // Phase overlays
    UTextBlock*       TxtPhase     = nullptr;  // PreWave, Gather (center)
    UTextBlock*       TxtCombat    = nullptr;  // Combat (top-center)
    UTextBlock*       TxtGameState = nullptr;  // GameOver, Victory (center, large)
    UTextBlock*       TxtRestart   = nullptr;

    // Shop panel
    UCanvasPanel*     PanelShop    = nullptr;
    UTextBlock*       TxtShopRes   = nullptr;

    static constexpr int32 MaxShopRows = 8;
    struct FShopRow
    {
        UTextBlock* Name   = nullptr;
        UTextBlock* Effect = nullptr;
        UTextBlock* Lvl    = nullptr;
        UTextBlock* Cost   = nullptr;
    };
    TArray<FShopRow> ShopRows;
};
