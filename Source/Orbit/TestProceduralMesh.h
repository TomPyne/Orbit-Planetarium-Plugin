// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealFastNoisePlugin/Public/FastNoise/FastNoise.h"
#include "TestProceduralMesh.generated.h"



UCLASS()
class ORBIT_API ATestProceduralMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestProceduralMesh();

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EFractalNoiseType NoiseType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	int32 Seed;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	int32 Octaves;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Frequency;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EFractalType FractalType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float FractalGain;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Lacunarity;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EInterp Interpolation;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float HeightBoost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Scale = 20.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Resolution = 20.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	UMaterial* DefaultMaterial;

	UPROPERTY(EditAnywhere, Category = "Output")
	UTexture2D* OutputTex;

	UFUNCTION(BlueprintCallable)
	static UTexture2D* Maketexture(TArray<FColor> colorMap, int size, UObject* outer);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateMesh(float width, float height);

	TArray<float> GetHeightDataFromTexture(const UTexture2D* inTexture);
	TArray<float> GetHeightDataFromAlgorithm();

	// Returns a value either sampling texture or noise generator
	float HeightValueAtCoord(int x, int y);

	// The height values stored in a linear array
	TArray<float> LinearHeightValues;

	UFastNoise* NoiseGen;
	class UUFNNoiseGenerator* NoiseGenerator;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GenerateNoiseMap();
	void CreateNoiseTexture(int texWidth, int texHeight);
	TArray<float>GetNoiseMap(int width, int height);
};
