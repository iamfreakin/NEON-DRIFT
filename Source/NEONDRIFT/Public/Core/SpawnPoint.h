#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonTypes.h"
#include "SpawnPoint.generated.h"

UCLASS()
class NEONDRIFT_API ASpawnPoint : public AActor
{
    GENERATED_BODY()
public:
    ASpawnPoint();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ESpawnDir Direction = ESpawnDir::North;
};
