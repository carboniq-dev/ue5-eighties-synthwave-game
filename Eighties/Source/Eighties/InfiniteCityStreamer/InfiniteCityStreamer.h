#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InfiniteCityStreamer.generated.h"

class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class EIGHTIES_API AInfiniteCityStreamer : public AActor
{
    GENERATED_BODY()

public:
    AInfiniteCityStreamer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Meshes")
    TArray<UStaticMesh*> MeshVariants;

    UPROPERTY(EditAnywhere, Category="Streaming", meta = (ClampMin = "0"))
    float SectionLength = 0.f;

    UPROPERTY(EditAnywhere, Category="Streaming")
    float VerticalOffset = 0.f;

    UPROPERTY(EditAnywhere, Category="Streaming", meta = (ClampMin = "1"))
    int32 LookAheadTiles = 15;

    UPROPERTY(EditAnywhere, Category="Streaming", meta = (ClampMin = "0"))
    int32 DropBehindTiles = 2;

    UPROPERTY(EditAnywhere, Category="Streaming")
    int32 RandomSeed = 12345;

private:
    struct FTile { int32 MeshID; int32 Inst; int32 Section; };
    TArray<FTile> TileRing;                    
    int32 BufferSize   = 0;
    int32 RingHead     = 0;                  
    int32 PlayerSection= 0;                    
    FVector Origin;                            
    FRandomStream RNG;

    bool   InitSectionLength();
    void   SpawnInitialWindow();
    void   AdvanceOne();
    void   MoveTrigger();
    FORCEINLINE FVector SectionPos(int32 S) const { return Origin + FVector(S * SectionLength, 0, 0); }


public:
    UFUNCTION()
    void OnTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditAnywhere, BlueprintReadWrite) UBoxComponent* Trigger;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<UHierarchicalInstancedStaticMeshComponent*> HISM;
};