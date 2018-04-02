// Tom Pyne - 2018

#include "SectionIcosahedron.h"
#include "RuntimeMeshComponent.h"


// Sets default values
ASectionIcosahedron::ASectionIcosahedron()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Mesh"));
	RootComponent = RuntimeMeshComponent;
}

// Called when the game starts or when spawned
void ASectionIcosahedron::BeginPlay()
{
	Super::BeginPlay();
	
	CreateIcosahedron();
}

void ASectionIcosahedron::CreateIcosahedron()
{
	float t = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;

	FVector origin = GetActorLocation();

	// Create the 12 vertices of the icosahedron
	FVector v0 = FVector(-1, t, 0);
	FVector v1 = FVector(1, t, 0);
	FVector v2 = FVector(-1, -t, 0);
	FVector v3 = FVector(1, -t, 0);

	FVector v4 = FVector(0, -1, t);
	FVector v5 = FVector(0, 1, t);
	FVector v6 = FVector(0, -1, -t);
	FVector v7 = FVector(0, 1, -t);

	FVector v8 = FVector(t, 0, -1);
	FVector v9 = FVector(t, 0, 1);
	FVector v10 = FVector(-t, 0, -1);
	FVector v11 = FVector(-t, 0, 1);

	FRuntimeMeshVertexSimple vs0 = FRuntimeMeshVertexSimple(-v0, v0 - origin);
	FRuntimeMeshVertexSimple vs1 = FRuntimeMeshVertexSimple(-v1, v1 - origin);
	FRuntimeMeshVertexSimple vs2 = FRuntimeMeshVertexSimple(-v2, v2 - origin);
	FRuntimeMeshVertexSimple vs3 = FRuntimeMeshVertexSimple(-v3, v3 - origin);
	FRuntimeMeshVertexSimple vs4 = FRuntimeMeshVertexSimple(-v4, v4 - origin);
	FRuntimeMeshVertexSimple vs5 = FRuntimeMeshVertexSimple(-v5, v5 - origin);
	FRuntimeMeshVertexSimple vs6 = FRuntimeMeshVertexSimple(-v6, v6 - origin);
	FRuntimeMeshVertexSimple vs7 = FRuntimeMeshVertexSimple(-v7, v7 - origin);
	FRuntimeMeshVertexSimple vs8 = FRuntimeMeshVertexSimple(-v8, v8 - origin);
	FRuntimeMeshVertexSimple vs9 = FRuntimeMeshVertexSimple(-v9, v9 - origin);
	FRuntimeMeshVertexSimple vs10 = FRuntimeMeshVertexSimple(-v10, v10 - origin);
	FRuntimeMeshVertexSimple vs11 = FRuntimeMeshVertexSimple(-v11, v11 - origin);

	// Initialise the section arrays
	TArray<TArray<int32>> sectionTris;
	sectionTris.Init(TArray<int32>(), 20);
	TArray<TArray<FRuntimeMeshVertexSimple>> sectionVerts;
	sectionVerts.Init(TArray<FRuntimeMeshVertexSimple>(), 20);

	// Create the 20 faces of the icosahedron as seperate mesh sections
	SetupSection(sectionVerts[0], sectionTris[0], vs0, vs11, vs5);
	SetupSection(sectionVerts[1], sectionTris[1], vs0, vs5, vs1);
	SetupSection(sectionVerts[2], sectionTris[2], vs0, vs1, vs7);
	SetupSection(sectionVerts[3], sectionTris[3], vs0, vs7, vs10);
	SetupSection(sectionVerts[4], sectionTris[4], vs0, vs10, vs11);

	SetupSection(sectionVerts[5], sectionTris[5], vs1, vs5, vs9);
	SetupSection(sectionVerts[6], sectionTris[6], vs5, vs11, vs4);
	SetupSection(sectionVerts[7], sectionTris[7], vs11, vs10, vs2);
	SetupSection(sectionVerts[8], sectionTris[8], vs10, vs7, vs6);
	SetupSection(sectionVerts[9], sectionTris[9], vs7, vs1, vs8);

	SetupSection(sectionVerts[10], sectionTris[10], vs3, vs9, vs4);
	SetupSection(sectionVerts[11], sectionTris[11], vs3, vs4, vs2);
	SetupSection(sectionVerts[12], sectionTris[12], vs3, vs2, vs6);
	SetupSection(sectionVerts[13], sectionTris[13], vs3, vs6, vs8);
	SetupSection(sectionVerts[14], sectionTris[14], vs3, vs8, vs9);

	SetupSection(sectionVerts[15], sectionTris[15], vs4, vs9, vs5);
	SetupSection(sectionVerts[16], sectionTris[16], vs2, vs4, vs11);
	SetupSection(sectionVerts[17], sectionTris[17], vs6, vs2, vs10);
	SetupSection(sectionVerts[18], sectionTris[18], vs8, vs6, vs7);
	SetupSection(sectionVerts[19], sectionTris[19], vs9, vs8, vs1);

	if (RuntimeMeshComponent)
	{
		for (int i = 0; i < 20; i++)
		{
			RuntimeMeshComponent->CreateMeshSection(i, sectionVerts[i], sectionTris[i]);
		}
	}

}

void ASectionIcosahedron::SetupSection(TArray<FRuntimeMeshVertexSimple>& outVertexList, TArray<int32> &outTriList, FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2)
{
	outVertexList.Add(vs0);
	outVertexList.Add(vs1);
	outVertexList.Add(vs2);
	outTriList.Add(0);
	outTriList.Add(1);
	outTriList.Add(2);
}

// Called every frame
void ASectionIcosahedron::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

