// Fill out your copyright notice in the Description page of Project Settings.

#include "Fountain.h"

// Sets default values
AFountain::AFountain()
{
	PrimaryActorTick.bCanEverTick = false;	// 틱함수 사용 여부. 사용하지 않으면 퍼포먼스 향상을 기대할 수 있음

	m_pBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BODY"));
	m_pWater = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WATER"));
	m_pLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LIGHT"));
	m_pSplash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SPLASH"));
	m_pRotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("ROTATINGMOVEMENT"));	// 액터는 틱함수가 실행되지 않아도 이 컴포넌트의 이동 메커니즘을 따른다. 

	RootComponent = m_pBody;				// 대표하는 컴포넌트가 필요. 
	m_pWater->SetupAttachment(m_pBody);
	m_pLight->SetupAttachment(m_pBody);
	m_pSplash->SetupAttachment(m_pBody);

	m_pWater->SetRelativeLocation(FVector(0.f, 0.f, 135.f));
	m_pLight->SetRelativeLocation(FVector(0.f, 0.f, 195.f));
	m_pSplash->SetRelativeLocation(FVector(0.f, 0.f, 195.f));

	// 에셋의 경로는 런타임 도중에 변경될 일이 없기 때문에 오브젝트가 생성될 때마다 생성자가 호출되어
	// 지역변수를 생성하고 초기화하는 작업은 불필요하다. 그래서 static으로 선언해서 한 번만 초기화하는 것이 바람직하다.

	// 각 컴포넌트가 불러오는 것은 같은 컴포넌트가 아니라, UStaticMesh, UParticleSystem등 리소스에 대한 정보기 때문에
	// 에셋을 찾을 때 Compoent가 붙여지지 않은 에셋을 찾는다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SM_BODY(TEXT("/Game/InfinityBladeGrassLands/Environments/Plains/Env_Plains_Ruins/StaticMesh/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01"));
	if (true == SM_BODY.Succeeded())
		m_pBody->SetStaticMesh(SM_BODY.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SM_WATER(TEXT("/Game/InfinityBladeGrassLands/Effects/FX_Meshes/Env/SM_Plains_Fountain_02.SM_Plains_Fountain_02"));
	if (true == SM_WATER.Succeeded())
		m_pWater->SetStaticMesh(SM_WATER.Object);

	static ConstructorHelpers::FObjectFinder<UParticleSystem>SM_SPLASH(TEXT("/Game/InfinityBladeGrassLands/Effects/FX_Ambient/Water/P_Water_Fountain_Splash_Base_01.P_Water_Fountain_Splash_Base_01"));
	if (true == SM_SPLASH.Succeeded())
		m_pSplash->SetTemplate(SM_SPLASH.Object);

	m_fRotateSpeed = 30.f;
	m_pRotatingMovement->RotationRate = FRotator(0.f, m_fRotateSpeed, 0.f);
}

void AFountain::PostInitializeComponents()
{
	Super::PostInitializeComponents();		// 액터 로직이 정상적으로 작동하기 위해서, 부모의 로직이 먼저 실행되어야한다. Super의 함수 실행 이후 현재 액터의 작업을 진행한다. 모든 오버라이딩된 함수 동일 적용
}

// Called when the game starts or when spawned
void AFountain::BeginPlay()
{
	Super::BeginPlay();
}

void AFountain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// 생성자의 틱 옵션을 false로 하면 실행되지 않는다.
void AFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);		
}

