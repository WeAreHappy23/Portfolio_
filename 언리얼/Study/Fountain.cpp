// Fill out your copyright notice in the Description page of Project Settings.

#include "Fountain.h"

// Sets default values
AFountain::AFountain()
{
	PrimaryActorTick.bCanEverTick = false;	// ƽ�Լ� ��� ����. ������� ������ �����ս� ����� ����� �� ����

	m_pBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BODY"));
	m_pWater = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WATER"));
	m_pLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LIGHT"));
	m_pSplash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SPLASH"));
	m_pRotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("ROTATINGMOVEMENT"));	// ���ʹ� ƽ�Լ��� ������� �ʾƵ� �� ������Ʈ�� �̵� ��Ŀ������ ������. 

	RootComponent = m_pBody;				// ��ǥ�ϴ� ������Ʈ�� �ʿ�. 
	m_pWater->SetupAttachment(m_pBody);
	m_pLight->SetupAttachment(m_pBody);
	m_pSplash->SetupAttachment(m_pBody);

	m_pWater->SetRelativeLocation(FVector(0.f, 0.f, 135.f));
	m_pLight->SetRelativeLocation(FVector(0.f, 0.f, 195.f));
	m_pSplash->SetRelativeLocation(FVector(0.f, 0.f, 195.f));

	// ������ ��δ� ��Ÿ�� ���߿� ����� ���� ���� ������ ������Ʈ�� ������ ������ �����ڰ� ȣ��Ǿ�
	// ���������� �����ϰ� �ʱ�ȭ�ϴ� �۾��� ���ʿ��ϴ�. �׷��� static���� �����ؼ� �� ���� �ʱ�ȭ�ϴ� ���� �ٶ����ϴ�.

	// �� ������Ʈ�� �ҷ����� ���� ���� ������Ʈ�� �ƴ϶�, UStaticMesh, UParticleSystem�� ���ҽ��� ���� ������ ������
	// ������ ã�� �� Compoent�� �ٿ����� ���� ������ ã�´�.
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
	Super::PostInitializeComponents();		// ���� ������ ���������� �۵��ϱ� ���ؼ�, �θ��� ������ ���� ����Ǿ���Ѵ�. Super�� �Լ� ���� ���� ���� ������ �۾��� �����Ѵ�. ��� �������̵��� �Լ� ���� ����
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

// �������� ƽ �ɼ��� false�� �ϸ� ������� �ʴ´�.
void AFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);		
}

