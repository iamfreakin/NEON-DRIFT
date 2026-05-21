#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NeonPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AManualTurret;
class APlayerShip;

UCLASS()
class NEONDRIFT_API ANeonPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ANeonPlayerController();

    // Input actions (public so Pawn can bind in SetupPlayerInputComponent)
    UPROPERTY() UInputMappingContext* IMC           = nullptr;
    UPROPERTY() UInputAction*         IA_MoveForward = nullptr;
    UPROPERTY() UInputAction*         IA_MoveRight   = nullptr;
    UPROPERTY() UInputAction*         IA_ThrustUp    = nullptr;
    UPROPERTY() UInputAction*         IA_ThrustDown  = nullptr;
    UPROPERTY() UInputAction*         IA_Look        = nullptr;
    UPROPERTY() UInputAction*         IA_Fire        = nullptr;
    UPROPERTY() UInputAction*         IA_Ready       = nullptr;
    UPROPERTY() UInputAction*         IA_ShopUp      = nullptr;
    UPROPERTY() UInputAction*         IA_ShopDown    = nullptr;
    UPROPERTY() UInputAction*         IA_ShopConfirm = nullptr;
    UPROPERTY() UInputAction*         IA_NextWave    = nullptr;
    UPROPERTY() UInputAction*         IA_Restart     = nullptr;
    UPROPERTY() UInputAction*         IA_Interact    = nullptr;
    UPROPERTY() UInputAction*         IA_Escape      = nullptr;

    UPROPERTY() AManualTurret* BoardedTurret = nullptr;

    bool  bPauseMenuOpen     = false;
    bool  bShowControlsPanel = false;
    int32 MenuCursorIndex    = 0;

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

    void OnShopUp();
    void OnShopDown();
    void OnShopConfirm();
    void OnNextWave();
    void OnRestart();
    void OnInteract();
    void OnEscapePressed();

    int32 ShopCursorIndex = 0;

private:
    void BuildIMC();
};
