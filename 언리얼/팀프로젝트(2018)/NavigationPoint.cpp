#include "NavigationPoint.h"
#include "Components/BoxComponent.h"

// 플레이어가 내비게이션 액터에 왔는지를 범위 오버랩으로 판단하도록 했습니다.
// 그러면 이벤트가 발생하여 네비게이션 액터는 다음 타겟으로 이동하게 됩니다.

ANavigationPoint::ANavigationPoint()
{

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetupAttachment(RootComponent);

	Collision->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));
	Collision->bGenerateOverlapEvents = true;
	Collision->SetCollisionProfileName("OverlapAll");

	Tags.Add(FName("NavigationEvent"));
}

void ANavigationPoint::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ANavigationPoint::OnOverlap);
}

void ANavigationPoint::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Character"))
	{
		NaviEvent.ExecuteIfBound();		// 이벤트 발생
	}
}