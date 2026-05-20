#include "SpawnPoint.h"
#include "Components/BillboardComponent.h"

ASpawnPoint::ASpawnPoint()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA 
	auto* Billboard = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	if (Billboard)
		Billboard->SetupAttachment(RootComponent);
#endif
}
