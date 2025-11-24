// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "VolumetricOctree.generated.h"

// --- Exercise 3: Boolean Operators ---
UENUM(BlueprintType)
enum class EGridOperation : uint8
{
    Union           UMETA(DisplayName = "Union (OR)"),
    Intersection    UMETA(DisplayName = "Intersection (AND)"),
};

// --- Exercise 5: Shape Types ---
UENUM(BlueprintType)
enum class EVolumetricShapeType : uint8
{
    Sphere  UMETA(DisplayName = "Sphere"),
    Box     UMETA(DisplayName = "Box"),
    Torus   UMETA(DisplayName = "Torus")
};

// --- Exercise 2 & 5: Defining Shapes ---
USTRUCT(BlueprintType)
struct FVolumetricShape
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EVolumetricShapeType Type = EVolumetricShapeType::Sphere;

    UPROPERTY(EditAnywhere)
    FVector Center = FVector::ZeroVector;

    // Dimensions:
    // For Sphere: X = Radius
    // For Box: X, Y, Z = Half-Extents (Length, Width, Height / 2)
    // For Torus: X = Major Radius (Ring size), Y = Minor Radius (Tube thickness)
    UPROPERTY(EditAnywhere)
    FVector Dimensions = FVector(150.0f, 150.0f, 150.0f);

    // Helper to get a bounding radius for initial culling
    float GetMaxRadius() const
    {
        return FMath::Max3(Dimensions.X, Dimensions.Y, Dimensions.Z) + (Type == EVolumetricShapeType::Torus ? Dimensions.X : 0);
    }
};

UCLASS()
class MODELLING3DTHREE_API AVolumetricOctree : public AActor
{
    GENERATED_BODY()
    
public:    
    AVolumetricOctree();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
    UInstancedStaticMeshComponent* VoxelISMC;

    // --- Settings ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings", meta=(ClampMin="1.0"))
    float MinVoxelSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    UStaticMesh* VoxelMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    TArray<FVolumetricShape> Shapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Settings")
    EGridOperation OperationMode;

    // --- Exercise 6: Tooling Data ---
    // Stores +1 (Add) or -1 (Remove) for specific grid indices
    TMap<FIntVector, float> SculptedModifications;

    // Function to apply the tool via code/blueprint
    UFUNCTION(BlueprintCallable, Category = "Voxel Tool")
    void ApplyTool(FVector ToolPosition, float ToolRadius, bool bAdd);

    // --- EDITOR DEBUG BUTTON FOR EXERCISE 6 ---
    // This allows you to test the tool directly in the details panel
    UPROPERTY(EditAnywhere, Category = "Exercise 6 Debug")
    FVector DebugToolLocation;
    
    UPROPERTY(EditAnywhere, Category = "Exercise 6 Debug")
    float DebugToolRadius = 50.0f;

    UFUNCTION(CallInEditor, Category = "Exercise 6 Debug")
    void DebugAddMatter();

    UFUNCTION(CallInEditor, Category = "Exercise 6 Debug")
    void DebugRemoveMatter();

    UFUNCTION(CallInEditor, Category = "Exercise 6 Debug")
    void ClearSculpting();

private:
    void GenerateOctree();
    void ProcessOctant(FVector Center, float Extent);
    
    // Core math logic
    bool DoesBoxIntersectShapes(const FBox& Box) const;
    bool IsPointInsideShapes(const FVector& P) const;
    
    // Helper for Exercise 5 math
    bool IsPointInsideSingleShape(const FVector& P, const FVolumetricShape& Shape) const;

    FIntVector GetGridIndex(FVector Position) const;
};