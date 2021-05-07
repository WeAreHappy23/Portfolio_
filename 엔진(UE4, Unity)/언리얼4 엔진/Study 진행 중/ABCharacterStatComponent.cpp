// Fill out your copyright notice in the Description page of Project Settings.

#include "ABGameInstance.h"
#include "ABCharacterStatComponent.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bWantsInitializeComponent = true;		// InitializeComponent함수가 호출하기 위해서는 이 값이 true가 되어야 함.

	m_iLevel = 1;
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// Called when the game starts
void UABCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetNewLevel(m_iLevel);
}


// Called every frame
void UABCharacterStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UABCharacterStatComponent::SetNewLevel(int32 _iLevel)
{
	auto pGameInstance = Cast<UABGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (nullptr == pGameInstance)
	{
		ABLOG(Warning, TEXT("Cannot Found GameInstance"));
		return;
	}

	m_ptStatData = pGameInstance->GetCharacterData(_iLevel);
	if (nullptr == m_ptStatData)
	{
		ABLOG(Warning, TEXT("Cannot Found Character Data"));
		return;
	}

	m_iLevel = _iLevel;
	m_fCurrentHP = m_ptStatData->GetMaxHP();
}

void UABCharacterStatComponent::Damaged(float _fAttack)
{
	if (nullptr == m_ptStatData)
	{
		ABLOG(Warning, TEXT("Cannot Found StatData"));
		return;
	}

	SetHP(FMath::Clamp(m_fCurrentHP - _fAttack, 0.f, m_ptStatData->GetMaxHP()));
}

void UABCharacterStatComponent::SetHP(float _fHP)
{
	m_fCurrentHP = _fHP;

	OnHPChangedDelegate.Broadcast();		// 위젯에 HP가 변화될 때 연결된 기능들을 호출

	if (KINDA_SMALL_NUMBER >= m_fCurrentHP)
		OnHPisZeroDelegate.Broadcast();		// 캐릭터의 HP가 없을 때 연결된 기능들을 호출
}

float UABCharacterStatComponent::GetDamage()
{
	return m_ptStatData->GetDamage();
}

float UABCharacterStatComponent::GetHPRatio()
{
	if (nullptr == m_ptStatData)
	{
		ABLOG(Warning, TEXT("Cannot Found Data"));
		return 0.f;
	}

	float fMaxHP = m_ptStatData->GetMaxHP();

	// 값 오류 시 로그 출력
	if (KINDA_SMALL_NUMBER >= fMaxHP)
	{
		ABLOG(Warning, TEXT("Value Error"));
		return 0.f;
	}

	return m_fCurrentHP / fMaxHP;
}

