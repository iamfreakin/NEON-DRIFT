#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NeonHUD.generated.h"

UCLASS()
class NEONDRIFT_API ANeonHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;

private:
    void DrawResources(float X, float Y);
    void DrawBaseHP(float X, float Y);
    void DrawPhaseInfo(float CX, float CY);
    void DrawShopMenu(float CX, float CY);

    UFont* GetFont() const;
};
