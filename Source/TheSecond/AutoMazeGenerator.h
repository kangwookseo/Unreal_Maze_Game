#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "AutoMazeGenerator.generated.h" // 이건 항상 마지막!

class UInstancedStaticMeshComponent;
class UStaticMeshComponent;

UCLASS()
class THESECOND_API AAutoMazeGenerator : public AActor
{
	GENERATED_BODY()

public:
	AAutoMazeGenerator();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void GenerateMaze();

	UFUNCTION(BlueprintCallable, Category = "Maze")
	FIntPoint GetStartCell() const 
	{ 
		return StartCell; 
	}

	UFUNCTION(BlueprintCallable, Category = "Maze")
	FIntPoint GetGoalCell() const 
	{ 
		return GoalCell; 
	}

protected:
	void VisitCell(int32 X, int32 Y, TArray<int32>& Grid);
	void ClearAllInstancesSafe();
	bool IsGridParamsValid() const;
	bool FindFarthestCellBFS(const TArray<int32>& Grid, const FIntPoint& InStart, FIntPoint& OutGoal) const;
	
	UFUNCTION()
	void OnGoalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnGoalReached_Bp();

public:
	// Grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = "5", UIMin = "5"))
	int32 Width = 21;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = "5", UIMin = "5"))
	int32 Height = 21;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = "100.0"))
	float TileSize = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	bool bGenerateOnBeginPlay = true;

	// Visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	FVector WallScale = FVector(2.f, 2.f, 4.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	FVector FloorScale = FVector(2.f, 2.f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	float WallZOffset = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	float FloorZOffset = 0.f;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	UInstancedStaticMeshComponent* WallISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	UInstancedStaticMeshComponent* FloorISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	UInstancedStaticMeshComponent* GoalFloorISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	UStaticMeshComponent* GoalMarker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze|Goal")
	UBoxComponent* GoalTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Goal")
	FVector GoalMarkerScale = FVector(1.f, 1.f, 2.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Goal")
	float GoalMarkerZOffset = 100.f;

private:
	FIntPoint StartCell = FIntPoint(1, 1);
	FIntPoint GoalCell = FIntPoint(1, 1);
};