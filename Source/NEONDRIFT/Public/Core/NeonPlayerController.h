#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NeonPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AManualTurret;

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
    UPROPERTY() UInputAction*         IA_Thrust      = nullptr;
    UPROPERTY() UInputAction*         IA_Look        = nullptr;
    UPROPERTY() UInputAction*         IA_Fire        = nullptr;
    UPROPERTY() UInputAction*         IA_Ready       = nullptr;
    UPROPERTY() UInputAction*         IA_ShopUp      = nullptr;
    UPROPERTY() UInputAction*         IA_ShopDown    = nullptr;
    UPROPERTY() UInputAction*         IA_ShopConfirm = nullptr;
    UPROPERTY() UInputAction*         IA_NextWave    = nullptr;
    UPROPERTY() UInputAction*         IA_Restart     = nullptr;

    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

    // UI input handlers (bound in BeginPlay)
    void OnShopUp();
    void OnShopDown();
    void OnShopConfirm();
    void OnNextWave();
    void OnRestart();

    int32 ShopCursorIndex = 0;

private:
    void BuildIMC();
};
