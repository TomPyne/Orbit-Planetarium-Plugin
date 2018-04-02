// Tom Pyne - 2018

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SectionIcosahedron.generated.h"

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
	float Radius = 10.0f;

	void CreateIcosahedron();

	void SetupSection(TArray<struct FRuntimeMeshVertexSimple> &outVertexList, TArray<int32> &outTriList, FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
