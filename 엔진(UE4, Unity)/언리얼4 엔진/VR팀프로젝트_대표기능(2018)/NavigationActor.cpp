#include "NavigationActor.h"
#include "Object/NavigationActor/NavigationPoint.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Object/NavigationActor/NavigationAIController.h"
#include "Particles/ParticleSystem.h"
#include "ParticleDefinitions.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

// 네비게이션 액터입니다.
// 플레이어가 포인트로 이동하면 이 내비게이션 액터는 다음 타깃으로 이동합니다.
// Current Point와 Target Point가 다를 때 행동 트리에서 다음 타겟으로 이동합니다.

// Sets default values
ANavigationActor::ANavigationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Scene->SetupAttachment(RootComponent);

	Navigate = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Navigate"));
	Navigate->SetupAttachment(Scene);

	// 시작과 끝 파티클 설정
	static ConstructorHelpers::FObjectFinder<UParticleSystem> P_Navigate(TEXT("ParticleSystem'/Game/Assets/Effect/Navigation/PS_GPP_Firefly.PS_GPP_Firefly'"));
	if (P_Navigate.Succeeded())
	{
		StartNavigate = P_Navigate.Object;
	}
	static ConstructorHelpers::FObjectFinder<UParticleSystem> P_EndNavigate(TEXT("ParticleSystem'/Game/Assets/Effect/Navigation/PS_GPP_Butterfly.PS_GPP_Butterfly'"));
	if (P_EndNavigate.Succeeded())
	{
		EndNavigate = P_EndNavigate.Object;
	}

	// 행동 트리를 찾습니다.
	static ConstructorHelpers::FObjectFinder<UBehaviorTree>Monster_BehaviorTree(TEXT("BehaviorTree'/Game/Blueprints/Object/Navigation/AI/BT_Navigation.BT_Navigation'"));
	if (Monster_BehaviorTree.Succeeded())
	{
		BehaviorTree = Monster_BehaviorTree.Object;
	}

	AIControllerClass = ANavigationAIController::StaticClass();

	// 현재 타겟과 목표 타겟
	CurrentPoint = -1;
	TargetPoint = -1;

	// 타겟 설정
	Target = NULL;

	Tags.Add(FName("Navigation"));
}

// Called when the game starts or when spawned
void ANavigationActor::BeginPlay()
{
	Super::BeginPlay();

	Navigate = UGameplayStatics::SpawnEmitterAttached(StartNavigate, RootComponent, NAME_None, GetActorLocation(), GetActorRotation(),
		EAttachLocation::KeepWorldPosition, false);

	// 이동할 수 있는 타겟이 있으면 이벤트에 등록합니다.
	if (Targets.Num() - 1 >= TargetPoint)
	{
		TargetPoint++;
		Target = Targets[TargetPoint];
		Register = Cast<ANavigationPoint>(Targets[TargetPoint]);
		Register->NaviEvent.BindUObject(this, &ANavigationActor::NavigationEvent);
	}
}

// Called every frame
void ANavigationActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AI = Cast<ANavigationAIController>(GetController());

	if (AI)
	{
		AI->BBComponent->SetValueAsInt("CurrentPoint", CurrentPoint);
		AI->BBComponent->SetValueAsInt("TargetPoint", TargetPoint);
		AI->BBComponent->SetValueAsObject("Target", Target);
		AI->BBComponent->SetValueAsBool("bIsSame", CurrentPoint == TargetPoint);
		AI->BBComponent->SetValueAsBool("bIsMax", TargetPoint == Targets.Num());
	}
}

// NavigationPoint.cpp 에서 이벤트를 호출하면 실행됩니다.
void ANavigationActor::NavigationEvent()
{
	CurrentPoint++;
	TargetPoint++;

	// 타겟 재설정 / 다음 타겟에 이벤트를 등록합니다.
	if (Targets.Num() > TargetPoint)
	{
		if (Targets[TargetPoint])
		{
			Target = Targets[TargetPoint];
			if (Register)
				Register->Collision->bGenerateOverlapEvents = false;
			Register = Cast<ANavigationPoint>(Targets[TargetPoint]);
			Register->NaviEvent.BindUObject(this, &ANavigationActor::NavigationEvent);
		}
	}

	// 마지막 타겟으로 이동 시 종료 이펙트를 실행시킵니다.
	if (CurrentPoint == Targets.Num() - 1)
	{
		Navigate->DeactivateSystem();

		if (EndNavigate)
			Navigate = UGameplayStatics::SpawnEmitterAttached(EndNavigate, RootComponent,
				NAME_None, GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
}

