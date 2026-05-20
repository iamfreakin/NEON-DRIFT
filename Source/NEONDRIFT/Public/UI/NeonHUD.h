#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NeonHUD.generated.h"

class UNeonHUDWidget;

UCLASS()
class NEONDRIFT_API ANeonHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

private:
    UPROPERTY() UNeonHUDWidget* HUDWidget = nullptr;
};
