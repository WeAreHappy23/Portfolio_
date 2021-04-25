#include "Lever.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"	
#include	"MyCharacter/MotionControllerCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HandMotionController/RightHandMotionController.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

#include"Components/BoxComponent.h"
#include"Components/StaticMeshComponent.h"

// 캐릭터의 손을 문의 Transform 기준으로 바꿔서 Atan2에서 각을 찾아서 Rotation에 적용했습니다.
// 밑변과 높이를 알 때, 라디안=atan(높이/밑변)이므로 각도를 얻을 수 있습니다.

ALever::ALever()
{
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Scene->SetupAttachment(RootComponent);

	LeverScene = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverScene"));
	LeverScene->SetupAttachment(Scene);

	Lever = CreateDefaultSubobject<UBoxComponent>(TEXT("Lever"));
	Lever->SetupAttachment(LeverScene);

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetupAttachment(Lever);

	// 스태틱 메쉬를 찾습니다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh>PotionShape(TEXT("StaticMesh'/Game/Assets/MapBuild/RoughMap/Bridge/mesh/bridge_door_bridge_door_01.bridge_door_bridge_door_01'"));
	if (PotionShape.Succeeded())
	{
		LeverScene->SetStaticMesh(PotionShape.Object);
	}

	// 각 컴포넌트의 크기와 위치를 설정합니다.
	LeverScene->SetRelativeScale3D(FVector(4.0f, 4.0f, 4.0f));
	Lever->SetRelativeLocation(FVector(-62.0f, 0.0f, 40.0f));
	Lever->SetRelativeScale3D(FVector(0.09f, 0.09f, 1.0f));
	Collision->SetRelativeLocation(FVector(360.0f, 0.0f, -40.0f));
	Collision->SetRelativeScale3D(FVector(10.0f, 0.7f, 2.6f));

	// 자동으로 열리는 위치입니다.
	AutoRot = FRotator(LeverScene->RelativeRotation.Pitch, LeverScene->RelativeRotation.Yaw + 60.0f, LeverScene->RelativeRotation.Roll);

	Collision->bGenerateOverlapEvents = false;

	// 각 컴포넌트의 콜리전 속성을 설정합니다.
	LeverScene->SetCollisionProfileName("NoCollision");		// 겹칠 때, 콜리전 없음
	Lever->SetCollisionProfileName("OverlapAll");			// 겹칠 때, 오버랩 이벤트
	Collision->SetCollisionProfileName("BlockAll");			// 겹칠 때, 블록

	Tags.Add(FName("Door"));
}

void ALever::BeginPlay()
{
	Super::BeginPlay();

	Lever->OnComponentBeginOverlap.AddDynamic(this, &ALever::OnLeverOverlap);
	Lever->OnComponentEndOverlap.AddDynamic(this, &ALever::OnLeverEndOverlap);
}

void ALever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 터치된 액터(컨트롤러)가 있을 때 열 수 있습니다.
	if (TouchActor)
	{
		ARightHandMotionController* RightHand = Cast<ARightHandMotionController>(TouchActor);
		if (RightHand)
		{
			if (RightHand->bisRightGrab)
			{
				// 손을 이 액터의 Transform으로 변환한 후 손의 위치에 따라 변화시킵니다.
				FVector Cal = UKismetMathLibrary::InverseTransformLocation
				(GetActorTransform(), RightHand->GetActorLocation());

				float degree = UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Atan2(-Cal.Y, -Cal.X));

				// 각도 적용
				LeverScene->SetRelativeRotation(FRotator(0.0f, degree, 0.0f));
			}
		}
	}

	// 각도가 10이상이 되면 특정 지점까지 자동으로 문이 열리도록 구현했습니다.
	if (LeverScene->RelativeRotation.Yaw > 10.0f)
	{
		LeverScene->SetRelativeRotation(FMath::Lerp(LeverScene->RelativeRotation, AutoRot, 0.05f));
	}

}

// 오버랩된 액터가 오른손일 때 Touch Actor에 값이 들어옵니다.
void ALever::OnLeverOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("RightHand"))
	{
		AMotionControllerCharacter* Character = Cast<AMotionControllerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (Character)
		{
			ARightHandMotionController* RightHand = Cast<ARightHandMotionController>(OtherActor);

			if (RightHand)
				TouchActor = Character->RightHand;
		}
	}
}