#include "InfiniteCityStreamer.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AInfiniteCityStreamer::AInfiniteCityStreamer()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    Trigger = CreateDefaultSubobject<UBoxComponent>("Trigger");
    Trigger->SetupAttachment(RootComponent);
    Trigger->SetMobility(EComponentMobility::Movable);          // ← critical line
    Trigger->SetBoxExtent(FVector(50.f, 5000.f, 5000.f));
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Overlap);
    Trigger->SetCollisionProfileName(TEXT("Trigger")); 
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AInfiniteCityStreamer::OnTrigger);
}

void AInfiniteCityStreamer::BeginPlay()
{
    Super::BeginPlay();

    HISM.Empty();

    for (UStaticMesh* Mesh : MeshVariants)
    {
        if (!Mesh) continue;

        auto* Comp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
        Comp->SetupAttachment(RootComponent);
        Comp->SetStaticMesh(Mesh);
        Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Comp->RegisterComponentWithWorld(GetWorld());  
        HISM.Add(Comp);
    }

    if (!InitSectionLength()) return;

    BufferSize = LookAheadTiles + DropBehindTiles + 1;
    TileRing.SetNum(BufferSize);
    RNG.Initialize(RandomSeed ? RandomSeed : FMath::Rand());

    const AActor* Anchor = Cast<AActor>(
        UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()));

    if (!Anchor) Anchor = UGameplayStatics::GetPlayerPawn(this, 0);

    const FVector StartLoc = Anchor ? Anchor->GetActorLocation()
                                    : FVector::ZeroVector;
    Origin = StartLoc + FVector(0, 0, VerticalOffset);

    if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        if (UPrimitiveComponent* Root =
                Cast<UPrimitiveComponent>(Pawn->GetRootComponent()))
        {
            Root->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        }
    }

    SpawnInitialWindow();
    MoveTrigger();
}

bool AInfiniteCityStreamer::InitSectionLength()
{
    if (SectionLength > KINDA_SMALL_NUMBER) return true;
    if (!MeshVariants.IsValidIndex(0) || !MeshVariants[0]) return false;

    const FBox Box = MeshVariants[0]->GetBoundingBox();
    SectionLength  = Box.GetSize().X;
    return SectionLength > KINDA_SMALL_NUMBER;
}

void AInfiniteCityStreamer::SpawnInitialWindow()
{
    int32 S = -DropBehindTiles;
    for (int32 i = 0; i < BufferSize; ++i, ++S)
    {
        const int32 M  = RNG.RandRange(0, MeshVariants.Num() - 1);
        const FTransform NewTransform(SectionPos(S));
        const int32 Inst = HISM[M]->AddInstance(NewTransform, true);

        TileRing[i] = { M, Inst, S };
    }
}

void AInfiniteCityStreamer::AdvanceOne()
{
    FTile& Old = TileRing[RingHead];
    const FTransform Hide(FVector(0, 0, -1000000.f));

    HISM[Old.MeshID]->UpdateInstanceTransform(
        Old.Inst, Hide, true, true, true);

    const int32 NewS = TileRing[(RingHead + BufferSize - 1) % BufferSize].Section + 1;
    const int32 M    = RNG.RandRange(0, MeshVariants.Num() - 1);
    const FTransform NewTransform(SectionPos(NewS));
    const int32 NewInst = HISM[M]->AddInstance(NewTransform, true);

    Old = { M, NewInst, NewS };
    RingHead = (RingHead + 1) % BufferSize;
}

void AInfiniteCityStreamer::MoveTrigger()
{
    Trigger->SetWorldLocation(SectionPos(PlayerSection + 1));
}

void AInfiniteCityStreamer::OnTrigger(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0)) return;

    const float PlayerX   = OtherActor->GetActorLocation().X - Origin.X;
    const int32 NewSecIdx = FMath::RoundToInt(PlayerX / SectionLength);
    const int32 Steps     = NewSecIdx - PlayerSection;
    
    if (Steps <= 0) return;

    for (int32 i = 0; i < Steps; ++i) AdvanceOne();
    
    PlayerSection = NewSecIdx;
    
    MoveTrigger();
}
