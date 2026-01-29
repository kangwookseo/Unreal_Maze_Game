#include "AutoMazeGenerator.h"

#include "Components/BoxComponent.h" 
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/Queue.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

AAutoMazeGenerator::AAutoMazeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WallISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISM"));
	WallISM->SetupAttachment(Root);
	WallISM->SetMobility(EComponentMobility::Static);

	FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
	FloorISM->SetupAttachment(Root);
	FloorISM->SetMobility(EComponentMobility::Static);

	GoalFloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GoalFloorISM"));
	GoalFloorISM->SetupAttachment(Root);
	GoalFloorISM->SetMobility(EComponentMobility::Static);

	WallISM->SetCanEverAffectNavigation(false);
	FloorISM->SetCanEverAffectNavigation(false);
	GoalFloorISM->SetCanEverAffectNavigation(false);

	GoalFloorISM->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	GoalFloorISM->SetGenerateOverlapEvents(true);

	GoalMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMarker"));
	GoalMarker->SetupAttachment(Root);
	GoalMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GoalMarker->SetMobility(EComponentMobility::Movable);
	GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));
	GoalTrigger->SetupAttachment(Root);
	GoalTrigger->SetBoxExtent(FVector(100.f, 100.f, 100.f)); // 박스 크기
	GoalTrigger->SetCollisionProfileName(TEXT("Trigger"));   // 겹침 감지용 프리셋
	GoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAutoMazeGenerator::OnGoalOverlap);
}

void AAutoMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateMaze();
	}
}

bool AAutoMazeGenerator::IsGridParamsValid() const
{
	if (Width < 3 || Height < 3) 
		return false;
	const int64 Num = (int64)Width * (int64)Height;
	if (Num <= 0 || Num > INT32_MAX) 
		return false;

	return true;
}

void AAutoMazeGenerator::ClearAllInstancesSafe()
{
	if (WallISM)
	{
		WallISM->ClearInstances();
	}
	if (FloorISM)
	{
		
		FloorISM->ClearInstances();
		
	}
	if (GoalFloorISM)
	{
		GoalFloorISM->ClearInstances();
	}
}

bool AAutoMazeGenerator::FindFarthestCellBFS(const TArray<int32>& Grid, const FIntPoint& InStart, FIntPoint& OutGoal) const
{
	auto Idx = [this](int32 X, int32 Y) { return X + Y * Width; };

	if (InStart.X < 0 || InStart.X >= Width || InStart.Y < 0 || InStart.Y >= Height) 
		return false;

	const int32 StartIdx = Idx(InStart.X, InStart.Y);

	if (!Grid.IsValidIndex(StartIdx) || Grid[StartIdx] != 0) 
		return false;

	TArray<int32> Dist;
	Dist.Init(-1, Width * Height);

	TQueue<FIntPoint> Q;
	Dist[StartIdx] = 0;
	Q.Enqueue(InStart);

	FIntPoint Best = InStart;
	int32 BestDist = 0;

	static const int32 DX[4] = { 1, -1, 0, 0 };
	static const int32 DY[4] = { 0, 0, 1, -1 };

	while (Q.IsEmpty() == false)
	{
		FIntPoint Cur;
		Q.Dequeue(Cur);

		const int32 CurIdx = Idx(Cur.X, Cur.Y);
		const int32 CurDist = Dist[CurIdx];

		if (CurDist > BestDist)
		{
			BestDist = CurDist;
			Best = Cur;
		}

		for (int32 k = 0; k < 4; ++k)
		{
			const int32 NX = Cur.X + DX[k];
			const int32 NY = Cur.Y + DY[k];

			if (NX < 0 || NX >= Width || NY < 0 || NY >= Height) 
				continue;

			const int32 NIdx = Idx(NX, NY);
			if (!Grid.IsValidIndex(NIdx) || Grid[NIdx] != 0 || Dist[NIdx] != -1) 
				continue;

			Dist[NIdx] = CurDist + 1;
			Q.Enqueue(FIntPoint(NX, NY));
		}
	}

	OutGoal = Best;
	return true;
}

void AAutoMazeGenerator::OnGoalOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)

{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		OnGoalReached_Bp();
	}
}

void AAutoMazeGenerator::GenerateMaze()
{
	// 1. 안전성 체크
	if (WallISM == nullptr || FloorISM == nullptr) 
		return;
	if (WallISM->GetStaticMesh() == nullptr || FloorISM->GetStaticMesh() == nullptr)
		return;

	// [중요] 짝수 크기면 홀수로 보정 (벽-길-벽 구조 유지를 위해 필수)
	if (Width % 2 == 0) 
		Width++;
	if (Height % 2 == 0) 
		Height++;

	if (IsGridParamsValid() == false)
		return;

	// 2. 초기화
	ClearAllInstancesSafe();

	TArray<int32> Grid;
	Grid.Init(1, Width * Height);

	// 3. 미로 데이터 생성
	StartCell = FIntPoint(1, 1);
	VisitCell(StartCell.X, StartCell.Y, Grid);

	// 4. 목표 지점 설정
	FIntPoint FoundGoal = StartCell;
	if (FindFarthestCellBFS(Grid, StartCell, FoundGoal))
	{
		GoalCell = FoundGoal;
	}
	else
	{
		GoalCell = StartCell;
	}

	// 5. 비주얼(메쉬) 배치 루프
	for (int32 x = 0; x < Width; ++x)
	{
		for (int32 y = 0; y < Height; ++y)
		{
			const int32 Index = x + y * Width;
			if (Grid.IsValidIndex(Index) == false) 
				continue;

			const FVector BaseLocation(x * TileSize, y * TileSize, 0.f);
			const bool bIsGoal = (x == GoalCell.X && y == GoalCell.Y);
		
			//벽
			if (Grid[Index] == 1) 
			{
				FTransform T;
				T.SetLocation(BaseLocation + FVector(0.f, 0.f, WallZOffset));
				T.SetRotation(FQuat::Identity);
				T.SetScale3D(WallScale);
				WallISM->AddInstance(T);
			}
			//바닥
			else 
			{
				FTransform T;
				T.SetLocation(BaseLocation + FVector(0.f, 0.f, FloorZOffset));
				T.SetRotation(FQuat::Identity);
				T.SetScale3D(FloorScale);

				if (bIsGoal && GoalFloorISM && GoalFloorISM->GetStaticMesh())
				{
					GoalFloorISM->AddInstance(T);
				}
				else
				{
					FloorISM->AddInstance(T);
				}
			}
		}
	}

	// 6. 렌더링 업데이트
	WallISM->MarkRenderStateDirty();
	FloorISM->MarkRenderStateDirty();
	if (GoalFloorISM) GoalFloorISM->MarkRenderStateDirty();

	FVector GoalWorld(GoalCell.X * TileSize, GoalCell.Y * TileSize, GoalMarkerZOffset);
	// 7. 목표 마커 이동
	if (GoalMarker)
	{
		GoalMarker->SetRelativeLocation(GoalWorld);
		GoalMarker->SetRelativeScale3D(GoalMarkerScale);
		GoalMarker->SetVisibility(GoalMarker->GetStaticMesh() != nullptr);
	}

	if (GoalTrigger)
	{
		GoalTrigger->SetRelativeLocation(GoalWorld);
	}

	// 1. 이동할 좌표 계산
	float StartX = StartCell.X * TileSize;
	float StartY = StartCell.Y * TileSize;
	float SpawnZ = FloorZOffset + 120.0f; // 바닥보다 살짝 위

	// 2. 월드 좌표로 변환
	FVector LocalStartPos = FVector(StartX, StartY, SpawnZ);
	FVector WorldStartPos = GetTransform().TransformPosition(LocalStartPos);

	// 3. 플레이어 이동
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		// 텔레포트로 강제 이동 시킵니다.
		PlayerPawn->SetActorLocation(WorldStartPos, false, nullptr, ETeleportType::TeleportPhysics);

		// (선택사항) 플레이어 회전도 미로 안쪽을 보게 초기화
		PlayerPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
	}
}
void AAutoMazeGenerator::VisitCell(int32 X, int32 Y, TArray<int32>& Grid)
{
	if (X < 0 || X >= Width || Y < 0 || Y >= Height) 
		return;
	const int32 Idx = X + Y * Width;
	if (Grid.IsValidIndex(Idx) == false) 
		return;

	Grid[Idx] = 0;

	TArray<int32> Directions = { 0, 1, 2, 3 };
	for (int32 i = 0; i < Directions.Num(); ++i)
	{
		const int32 SwapIdx = FMath::RandRange(i, Directions.Num() - 1);
		if (i != SwapIdx) 
			Directions.Swap(i, SwapIdx);
	}

	for (const int32 Dir : Directions)
	{
		int32 NextX = X;
		int32 NextY = Y;

		switch (Dir)
		{
		case 0: NextY += 2; 
			break;
		case 1: NextY -= 2; 
			break;
		case 2: NextX -= 2;
			break;
		case 3: NextX += 2;
			break;
		default: 
			break;
		}

		if (NextX > 0 && NextX < Width - 1 && NextY > 0 && NextY < Height - 1)
		{
			const int32 NextIdx = NextX + NextY * Width;
			if (Grid.IsValidIndex(NextIdx) == false) 
				continue;

			if (Grid[NextIdx] == 1)
			{
				const int32 WallX = X + (NextX - X) / 2;
				const int32 WallY = Y + (NextY - Y) / 2;
				const int32 WallIdx = WallX + WallY * Width;

				if (Grid.IsValidIndex(WallIdx))
				{
					Grid[WallIdx] = 0;
				}

				VisitCell(NextX, NextY, Grid);
			}
		}
	}
}
