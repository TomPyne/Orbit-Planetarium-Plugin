// Tom Pyne - 2018

#include "SectionIcosahedron.h"



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

	// Create the 12 vertices of the icosahedron
	FVector v0 = FVector(1, -t, 0);
	FVector v1 = FVector(-1, -t, 0);
	FVector v2 = FVector(1, t, 0);
	FVector v3 = FVector(-1, t, 0);
	FVector v4 = FVector(0, 1, -t);
	FVector v5 = FVector(0, -1, -t);
	FVector v6 = FVector(0, 1, t);
	FVector v7 = FVector(0, -1, t);
	FVector v8 = FVector(-t, 0, 1);
	FVector v9 = FVector(-t, 0, -1);
	FVector v10 = FVector(t, 0, 1);
	FVector v11 = FVector(t, 0, -1);

	// Convert the, to the packed version with normal data
	FRuntimeMeshVertexSimple vs0 = FRuntimeMeshVertexSimple(v0, v0);
	FRuntimeMeshVertexSimple vs1 = FRuntimeMeshVertexSimple(v1, v1);
	FRuntimeMeshVertexSimple vs2 = FRuntimeMeshVertexSimple(v2, v2);
	FRuntimeMeshVertexSimple vs3 = FRuntimeMeshVertexSimple(v3, v3);
	FRuntimeMeshVertexSimple vs4 = FRuntimeMeshVertexSimple(v4, v4);
	FRuntimeMeshVertexSimple vs5 = FRuntimeMeshVertexSimple(v5, v5);
	FRuntimeMeshVertexSimple vs6 = FRuntimeMeshVertexSimple(v6, v6 );
	FRuntimeMeshVertexSimple vs7 = FRuntimeMeshVertexSimple(v7, v7);
	FRuntimeMeshVertexSimple vs8 = FRuntimeMeshVertexSimple(v8, v8);
	FRuntimeMeshVertexSimple vs9 = FRuntimeMeshVertexSimple(v9, v9);
	FRuntimeMeshVertexSimple vs10 = FRuntimeMeshVertexSimple(v10, v10);
	FRuntimeMeshVertexSimple vs11 = FRuntimeMeshVertexSimple(v11, v11);


	// Initialise the section arrays
	
	Sections.Empty();
	Sections.Init(NewObject<USectionData>(), 20);

	// init
	Sections.Empty();
	for (int i = 0; i < 20; i++)
	{
		Sections.Add(NewObject<USectionData>());
	}

	// Create the 20 faces of the icosahedron as seperate mesh sections
	SetupSection(Sections[0], vs0, vs11, vs5);
	SetupSection(Sections[1], vs0, vs5, vs1);
	SetupSection(Sections[2], vs0, vs1, vs7);
	SetupSection(Sections[3], vs0, vs7, vs10);
	SetupSection(Sections[4], vs0, vs10, vs11);

	SetupSection(Sections[5], vs1, vs5, vs9);
	SetupSection(Sections[6], vs5, vs11, vs4);
	SetupSection(Sections[7], vs11, vs10, vs2);
	SetupSection(Sections[8], vs10, vs7, vs6);
	SetupSection(Sections[9], vs7, vs1, vs8);

	SetupSection(Sections[10], vs3, vs9, vs4);
	SetupSection(Sections[11], vs3, vs4, vs2);
	SetupSection(Sections[12], vs3, vs2, vs6);
	SetupSection(Sections[13], vs3, vs6, vs8);
	SetupSection(Sections[14], vs3, vs8, vs9);

	SetupSection(Sections[15], vs4, vs9, vs5);
	SetupSection(Sections[16], vs2, vs4, vs11);
	SetupSection(Sections[17], vs6, vs2, vs10);
	SetupSection(Sections[18], vs8, vs6, vs7);
	SetupSection(Sections[19], vs9, vs8, vs1);

	float sTime = FPlatformTime::Seconds();

	// Refine sections
	for (int i = 0; i < Sections.Num() - 1; i++)
	{
		
		SubdivideMeshSection(Sections[i], FMath::RandRange(1, Subdivisions));
	}

	SubdivideMeshSection(Sections[19], 10);
	
	UE_LOG(LogTemp, Warning, TEXT("Time to subd mesh: %f"), (FPlatformTime::Seconds() - sTime));
	sTime = FPlatformTime::Seconds();

	if (RuntimeMeshComponent)
	{
		for (int i = 0; i < 20; i++)
		{
			RuntimeMeshComponent->CreateMeshSection(i, Sections[i]->Vertices, Sections[i]->Triangles);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Time to generate mesh: %f"), (FPlatformTime::Seconds() - sTime));
}

void ASectionIcosahedron::SetupSection(USectionData* SectionData, FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2)
{
	SectionData->Vertices.Add(vs0);
	SectionData->Vertices.Add(vs1);
	SectionData->Vertices.Add(vs2);
	SectionData->Triangles.Add(0);
	SectionData->Triangles.Add(1);
	SectionData->Triangles.Add(2);
}

// Called every frame
void ASectionIcosahedron::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int ASectionIcosahedron::GetMiddlePoint(int p1, int p2, USectionData* SectionData)
{
	if (SectionData == nullptr) { return 0; }

	// Calculate it
	FVector point1 = SectionData->Vertices[p1].Position;
	FVector point2 = SectionData->Vertices[p2].Position;
	FVector middle = FVector(
		(point1.X + point2.X) / 2.0f,
		(point1.Y + point2.Y) / 2.0f,
		(point1.Z + point2.Z) / 2.0f);

	int i = AddVertex(middle, SectionData);
	return i;
}

int ASectionIcosahedron::AddVertex(FVector v, USectionData* SectionData)
{
	if (SectionData == nullptr) { return 0; }

	double Length = v.Size();
	SectionData->Vertices.Add(FRuntimeMeshVertexSimple( FVector(v.X, v.Y, v.Z), FVector(v.X, v.Y, v.Z)));
	return SectionData->Index++;
}

void ASectionIcosahedron::SubdivideMeshSection(USectionData* SectionData, int recursion)
{
	if (SectionData == nullptr) { return; }

	for (int i = 0; i < recursion; i++)
	{
		int tris = SectionData->Triangles.Num();
		TArray<int32> tris2;
		for (int j = 0; j < tris; j = j + 3)
		{
			int v1 = SectionData->Triangles[j];
			int v2 = SectionData->Triangles[j + 1];
			int v3 = SectionData->Triangles[j + 2];
			int a = GetMiddlePoint(v1, v2, SectionData);
			int b = GetMiddlePoint(v2, v3, SectionData);
			int c = GetMiddlePoint(v3, v1, SectionData);

			tris2.Add(v1);
			tris2.Add(a);
			tris2.Add(c);
			
			tris2.Add(v2);
			tris2.Add(b);
			tris2.Add(a);

			tris2.Add(v3);
			tris2.Add(c);
			tris2.Add(b);

			tris2.Add(a);
			tris2.Add(b);
			tris2.Add(c);
		}

		SectionData->Triangles = tris2;

	}
}

