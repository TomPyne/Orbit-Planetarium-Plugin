// Tom Pyne - 2018

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"
#include "SectionIcosahedron.generated.h"

UCLASS()
class USectionData : public UObject
{
	GENERATED_BODY()

public:

	USectionData() {}

	TArray<int32> Triangles;
	TArray<FRuntimeMeshVertexSimple> Vertices;

	int Index = 3;	
};

UCLASS()
class ORBIT_API ASectionIcosahedron : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASectionIcosahedron();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class URuntimeMeshComponent* RuntimeMeshComponent;

	UPROPERTY(EditAnywhere)
	int Subdivisions = 4;

	void CreateIcosahedron();

	void SetupSection(USectionData* SectionData, FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2);

	void SubdivideMeshSection(USectionData* SectionData, int recursion);

	static int GetMiddlePoint(int p1, int p2, USectionData* SectionData);
	static int AddVertex(FVector v, USectionData* SectionData);

	UPROPERTY()
	TArray<USectionData* > Sections;

	TArray<TArray<int32>> SectionTris;
	TArray<TArray<FRuntimeMeshVertexSimple>> SectionVerts;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
