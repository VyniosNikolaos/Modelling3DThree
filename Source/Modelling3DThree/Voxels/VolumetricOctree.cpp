#include "VolumetricOctree.h"

AVolumetricOctree::AVolumetricOctree()
{
    PrimaryActorTick.bCanEverTick = false;

    VoxelISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VoxelISMC"));
    RootComponent = VoxelISMC;
    VoxelISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MinVoxelSize = 20.0f;
    OperationMode = EGridOperation::Union;

    // Default: One Sphere
    FVolumetricShape DefaultSphere;
    DefaultSphere.Type = EVolumetricShapeType::Sphere;
    DefaultSphere.Dimensions = FVector(200.0f, 0.0f, 0.0f); // X is Radius
    Shapes.Add(DefaultSphere);
}

void AVolumetricOctree::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (VoxelMesh) VoxelISMC->SetStaticMesh(VoxelMesh);
    GenerateOctree();
}

void AVolumetricOctree::GenerateOctree()
{
    if (Shapes.Num() == 0 || MinVoxelSize < 1.0f) return;
    VoxelISMC->ClearInstances();

    // 1. Determine Global Bounds
    FBox GlobalBounds(EForceInit::ForceInit);
    for (const FVolumetricShape& Shape : Shapes)
    {
        FVector Extent = FVector::ZeroVector;
        
        // Approximate bounds for Ex 5 shapes
        if (Shape.Type == EVolumetricShapeType::Sphere) {
            Extent = FVector(Shape.Dimensions.X);
        }
        else if (Shape.Type == EVolumetricShapeType::Box) {
            Extent = Shape.Dimensions;
        }
        else if (Shape.Type == EVolumetricShapeType::Torus) {
            // Torus Extent = MajorRadius + MinorRadius
            float TotalR = Shape.Dimensions.X + Shape.Dimensions.Y;
            Extent = FVector(TotalR, TotalR, Shape.Dimensions.Y);
        }

        GlobalBounds += (Shape.Center + Extent);
        GlobalBounds += (Shape.Center - Extent);
    }

    // Ensure we also cover any sculpted areas (Ex 6) so they don't disappear if outside original bounds
    if (SculptedModifications.Num() > 0)
    {
        // Simply expand bounds by a safe amount to catch sculptures, 
        // or iterate through keys (slower)
        GlobalBounds = GlobalBounds.ExpandBy(500.0f);
    }

    // Octree Root Setup
    FVector Size = GlobalBounds.GetSize();
    float MaxDim = FMath::Max3(Size.X, Size.Y, Size.Z);
    FVector RootCenter = GlobalBounds.GetCenter();
    float RootExtent = MaxDim * 0.6f;

    ProcessOctant(RootCenter, RootExtent);
}

void AVolumetricOctree::ProcessOctant(FVector Center, float Extent)
{
    FBox NodeBox(Center - FVector(Extent), Center + FVector(Extent));

    // Optimization: Cull empty space
    if (!DoesBoxIntersectShapes(NodeBox)) return;

    // Base Case: Leaf Node
    if (Extent <= (MinVoxelSize * 0.51f))
    {
        if (IsPointInsideShapes(Center))
        {
            FTransform T;
            T.SetLocation(Center);
            T.SetScale3D(FVector((Extent * 2.0f) / 100.0f)); 
            VoxelISMC->AddInstance(T);
        }
        return;
    }

    // Recursive Step
    float ChildExtent = Extent * 0.5f;
    for (int32 z = -1; z <= 1; z += 2)
    {
        for (int32 y = -1; y <= 1; y += 2)
        {
            for (int32 x = -1; x <= 1; x += 2)
            {
                FVector ChildOffset(x * ChildExtent, y * ChildExtent, z * ChildExtent);
                ProcessOctant(Center + ChildOffset, ChildExtent);
            }
        }
    }
}

// --- Exercise 5: Geometric Logic ---

bool AVolumetricOctree::IsPointInsideSingleShape(const FVector& P, const FVolumetricShape& Shape) const
{
    FVector LocalP = P - Shape.Center; // Simple relative position

    switch (Shape.Type)
    {
        case EVolumetricShapeType::Sphere:
        {
            // x^2 + y^2 + z^2 <= r^2
            return LocalP.SizeSquared() <= (Shape.Dimensions.X * Shape.Dimensions.X);
        }
        case EVolumetricShapeType::Box:
        {
            // |x| < SizeX && |y| < SizeY ...
            return (FMath::Abs(LocalP.X) <= Shape.Dimensions.X &&
                    FMath::Abs(LocalP.Y) <= Shape.Dimensions.Y &&
                    FMath::Abs(LocalP.Z) <= Shape.Dimensions.Z);
        }
        case EVolumetricShapeType::Torus:
        {
            // Torus formula: (MajorR - sqrt(x^2 + y^2))^2 + z^2 <= MinorR^2
            float MajorR = Shape.Dimensions.X;
            float MinorR = Shape.Dimensions.Y;
            
            float DistToAxis = FMath::Sqrt(LocalP.X * LocalP.X + LocalP.Y * LocalP.Y);
            float DistTube = FMath::Square(DistToAxis - MajorR) + (LocalP.Z * LocalP.Z);
            
            return DistTube <= (MinorR * MinorR);
        }
    }
    return false;
}

bool AVolumetricOctree::IsPointInsideShapes(const FVector& P) const
{
    // 1. Check Tooling (Exercise 6) - Modifiers override Geometry
    FIntVector Idx = GetGridIndex(P);
    if (const float* Mod = SculptedModifications.Find(Idx))
    {
        if (*Mod < -0.5f) return false; // Force Empty
        if (*Mod > 0.5f) return true;   // Force Solid
    }

    // 2. Check Geometry
    bool bInShapes = (OperationMode == EGridOperation::Intersection); // Default for AND is true, OR is false

    for (const FVolumetricShape& Shape : Shapes)
    {
        bool bInsideCurrent = IsPointInsideSingleShape(P, Shape);

        if (OperationMode == EGridOperation::Union)
        {
            if (bInsideCurrent) return true; // Inside ANY -> True
        }
        else // Intersection
        {
            if (!bInsideCurrent) return false; // Outside ANY -> False
        }
    }

    // Result for Intersection if loop finishes
    return (OperationMode == EGridOperation::Intersection);
}

bool AVolumetricOctree::DoesBoxIntersectShapes(const FBox& Box) const
{
    // Important: Also check if box intersects "Sculpted" positive zones
    // (This is a simplified check: if we have sculptures, we assume we need to check everything 
    // to be safe, or we implement a complex bounds check for the map).
    if (SculptedModifications.Num() > 0) return true; 

    // Geometric Check
    if (OperationMode == EGridOperation::Union)
    {
        for (const FVolumetricShape& Shape : Shapes)
        {
            // Simplified: Check Box against Shape Bounding Box
            // A perfect implementation would use Shape-Specific Box Intersection
            float Radius = Shape.GetMaxRadius();
            FBox ShapeBounds = FBox(Shape.Center - FVector(Radius), Shape.Center + FVector(Radius));
            
            if (Box.Intersect(ShapeBounds)) return true;
        }
        return false;
    }
    else // Intersection
    {
        // For intersection, the box must touch ALL shape bounds
        for (const FVolumetricShape& Shape : Shapes)
        {
            float Radius = Shape.GetMaxRadius();
            FBox ShapeBounds = FBox(Shape.Center - FVector(Radius), Shape.Center + FVector(Radius));
            
            if (!Box.Intersect(ShapeBounds)) return false; 
        }
        return true;
    }
}

// --- Exercise 6: Tooling & Debug ---

FIntVector AVolumetricOctree::GetGridIndex(FVector Position) const
{
    return FIntVector(
        FMath::RoundToInt(Position.X / MinVoxelSize),
        FMath::RoundToInt(Position.Y / MinVoxelSize),
        FMath::RoundToInt(Position.Z / MinVoxelSize)
    );
}

void AVolumetricOctree::ApplyTool(FVector ToolPosition, float ToolRadius, bool bAdd)
{
    int32 Range = FMath::CeilToInt(ToolRadius / MinVoxelSize);
    FIntVector CenterIdx = GetGridIndex(ToolPosition);

    bool bChanged = false;

    for (int z = -Range; z <= Range; z++)
    {
        for (int y = -Range; y <= Range; y++)
        {
            for (int x = -Range; x <= Range; x++)
            {
                FVector VoxelPos;
                VoxelPos.X = (CenterIdx.X + x) * MinVoxelSize;
                VoxelPos.Y = (CenterIdx.Y + y) * MinVoxelSize;
                VoxelPos.Z = (CenterIdx.Z + z) * MinVoxelSize;

                if (FVector::Dist(VoxelPos, ToolPosition) <= ToolRadius)
                {
                    FIntVector Key = GetGridIndex(VoxelPos);
                    SculptedModifications.Add(Key, bAdd ? 1.0f : -1.0f);
                    bChanged = true;
                }
            }
        }
    }

    if (bChanged)
    {
        GenerateOctree(); // Re-build mesh to show changes
    }
}

// Editor Debug Wrapper Functions
void AVolumetricOctree::DebugAddMatter()
{
    ApplyTool(GetActorLocation() + DebugToolLocation, DebugToolRadius, true);
}

void AVolumetricOctree::DebugRemoveMatter()
{
    ApplyTool(GetActorLocation() + DebugToolLocation, DebugToolRadius, false);
}

void AVolumetricOctree::ClearSculpting()
{
    SculptedModifications.Empty();
    GenerateOctree();
}