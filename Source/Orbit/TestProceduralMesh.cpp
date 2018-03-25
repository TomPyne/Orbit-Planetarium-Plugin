// Fill out your copyright notice in the Description page of Project Settings.

#include "TestProceduralMesh.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "FastNoise.h"
#include "UnrealFastNoisePlugin/Public/UFNNoiseGenerator.h"
#include "UnrealFastNoisePlugin/Public/UFNBlueprintFunctionLibrary.h"

USTRUCT()
struct FMeshData
{

	int32 TriangleIndex;

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV;
	TArray<FVector> Normals;

	FMeshData() {}

	FMeshData(int width, int height)
	{
		Vertices.SetNum(width * height);
		UV.SetNum(width * height);
		//Triangles.SetNum((width - 1)*(height - 1) * 6);
	}

	void AddTriangle(int a, int b, int c)
	{
		Triangles.Add(a);
		Triangles.Add(b);
		Triangles.Add(c);
		/*Triangles[TriangleIndex] = a;
		Triangles[TriangleIndex + 1] = b;
		Triangles[TriangleIndex + 2] = c;
		TriangleIndex += 3;*/
	}

	void CreateMesh(UProceduralMeshComponent* mesh)
	{
		if (!mesh) return;
		mesh->ClearAllMeshSections();

		mesh->bUseAsyncCooking = true;

		TArray<FColor> vertexColors;
		for (int i = 0; i < Vertices.Num(); i++)
			vertexColors.Add(FColor(255, 255, 255));

		TArray<FProcMeshTangent> tangents;

		mesh->CreateMeshSection(0, Vertices, Triangles, Normals , UV, vertexColors, tangents, false );
	}
};


// Sets default values
ATestProceduralMesh::ATestProceduralMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}



// Called when the game starts or when spawned
void ATestProceduralMesh::BeginPlay()
{
	Super::BeginPlay();

	if (OutputTex != nullptr)
	{
		LinearHeightValues = GetHeightDataFromTexture(OutputTex);
	}
	else
	{
		LinearHeightValues = GetHeightDataFromAlgorithm();
	}

	GenerateNoiseMap();
	CreateNoiseTexture(Resolution, Resolution);
	GenerateMesh(Resolution, Resolution);
}

void ATestProceduralMesh::GenerateMesh(float width, float height)
{
	FMeshData meshData = FMeshData(width, height);
	int vertexIndex = 0;
	float topLeftX = (width - 1) / -2.0f;
	float topLeftY = (height - 1) / 2.0f;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			FVector vertex = FVector(topLeftX + x, y, NoiseGenerator->GetNoise2D(x, topLeftY - y) * HeightBoost);
			vertex *= Scale;
			meshData.Vertices[vertexIndex] = vertex;
			meshData.UV[vertexIndex] = FVector2D((x / (float)width), (y / (float)height));

			if (x < width - 1 && y < height - 1)
			{
				meshData.AddTriangle(vertexIndex, vertexIndex + width + 1, vertexIndex + width);
				meshData.AddTriangle(vertexIndex + width + 1, vertexIndex, vertexIndex + 1);
			}

			vertexIndex++;
		}
	}
	UProceduralMeshComponent* mesh = FindComponentByClass<UProceduralMeshComponent>();
	if (mesh)
	{
		meshData.CreateMesh(mesh);
		
		if(DefaultMaterial) mesh->SetMaterial(0, DefaultMaterial);
	}

		
}

TArray<float> ATestProceduralMesh::GetHeightDataFromTexture(const UTexture2D * inTexture)
{
	if (inTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bad Texture passed to ATestProceduralMesh::GetHeightDataFromTexture"));
		return TArray<float>();
	}
	if (!FMath::IsPowerOfTwo(inTexture->GetSizeX() * inTexture->GetSizeY()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Texture passed to ATestProceduralMesh::GetHeightDataFromTexture is not a power of 2 texture"));
		return TArray<float>();
	}

	FTexture2DMipMap* mipMap = &inTexture->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &mipMap->BulkData;

	FColor* imageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
	TArray<float> floatValues;

	for (int i = 0; i < (mipMap->SizeX - 1) * (mipMap->SizeY - 1); i++)
	{
		floatValues.Add(imageData[i].A);
	}

	RawImageData->Unlock();

	return floatValues;
}

TArray<float> ATestProceduralMesh::GetHeightDataFromAlgorithm()
{
	TArray<float> heightData;
	if (NoiseGenerator == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Noise Generator created, ATestProceduralMesh::GetHeightDataFromAlgorithm"));
	}

	for (int x = 0; x < Resolution; x++)
	{
		for (int y = 0; y < Resolution; y++)
		{
			heightData.Add(NoiseGenerator->GetNoise2D(x, y));
		}
	}

	return heightData;
}

float ATestProceduralMesh::HeightValueAtCoord(int x, int y)
{
	int index = (y * Resolution) + x;
	if (LinearHeightValues.Num() > index)
	{
		return LinearHeightValues[index];
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to access coordinate not on the texture, returning 0, ATestProceduralMesh::HeightValueAtCoord"));
		return 0.0f;
	}
}

void ATestProceduralMesh::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

}

// Called every frame
void ATestProceduralMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestProceduralMesh::GenerateNoiseMap()
{
	NoiseGenerator = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
		this,
		NoiseType,
		Seed,
		Frequency,
		FractalGain,
		Interpolation,
		FractalType,
		Octaves,
		Lacunarity
	);

	UFastNoise* noise = NewObject<UFastNoise>(this);

	switch (NoiseType)
	{
	case EFractalNoiseType::FractalGradient:
		noise->SetNoiseType(ENoiseType::GradientFractal);
		break;
	case EFractalNoiseType::FractalSimplex:
		noise->SetNoiseType(ENoiseType::SimplexFractal);
		break;
	case EFractalNoiseType::FractalValue:
		noise->SetNoiseType(ENoiseType::ValueFractal);
		break;
	}

	noise->SetSeed(Seed);
	noise->SetFractalOctaves(Octaves);
	noise->SetFrequency(Frequency);
	noise->SetFractalType(FractalType);
	noise->SetFractalGain(FractalGain);
	noise->SetFractalLacunarity(Lacunarity);
	noise->SetInterp(Interpolation);

	NoiseGen = noise;
}

void ATestProceduralMesh::CreateNoiseTexture(int texWidth, int texHeight)
{
	FCreateTexture2DParameters params;
	TArray<float> floatMap = GetNoiseMap(texWidth, texHeight);
	TArray<FColor> colorMap;
	for (float f : floatMap)
	{
		colorMap.Add(FColor(f * 255, f * 255, f * 255, 255));
	}

	OutputTex = FImageUtils::CreateTexture2D(texWidth,
		texHeight,
		colorMap,
		this,
		TEXT("Noise"),
		EObjectFlags::RF_Transient,
		params);
	float bingle = 10;
}

UTexture2D * ATestProceduralMesh::Maketexture(TArray<FColor> colorMap, int size, UObject* outer)
{
	FCreateTexture2DParameters params;
	return FImageUtils::CreateTexture2D(size, size,
		colorMap, outer, TEXT("Noise"),
		EObjectFlags::RF_Transient,
		params);
}

TArray<float> ATestProceduralMesh::GetNoiseMap(int width, int height)
{	
	TArray<float> map;
	if (!NoiseGen) return map;

	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)		
		{
			map.Add(NoiseGenerator->GetNoise2D(x, y));
		}




	return map;
}




