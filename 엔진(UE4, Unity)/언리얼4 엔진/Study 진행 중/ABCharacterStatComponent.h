// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Components/ActorComponent.h"
#include "ABCharacterStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHPisZeroDelegate);
DECLARE_MULTICAST_DELEGATE(FOnHPChangedDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENABATTLE_API UABCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UABCharacterStatComponent();

private:
	// Transient 키워드
	//  UObject에는 직렬화 기능이 있어 UPROPERTY속성을 세이브/로드할 수 있다. 하지만 CurrentHP와 같이
	// 게임을 시작할 때 마다 변경되는 값을 저장하는 것은 의미가 없고 불필요한 디스크 공간을 차지한다. Transient는 직렬화 대상으로 하지 않겠다는 키워드

	struct FABCharacterData*	m_ptStatData = nullptr;		// 헤더파일을 추가하지 않기 위해서 class처럼 즉석에서 struct 전방선언을하고 포인터를 사용
	UPROPERTY(EditInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess))
		int32			m_iLevel;
	UPROPERTY(Transient, VisibleInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess))
		float			m_fCurrentHP;

public:
	FOnHPisZeroDelegate		OnHPisZeroDelegate;
	FOnHPChangedDelegate		OnHPChangedDelegate;

protected:
	virtual void InitializeComponent() override;		// 액터의 PostInitializeComponents에 대응하는 함수. PostInitializeComponents함수보다 먼저 호출
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetNewLevel(int32 _iLevel);	// 레벨 설정
	void Damaged(float _fAttack);		// 매개변수의 값으로 데미지를 받는 함수
	void SetHP(float _fHP);	// 매개변수의 값으로 HP를 설정
	float GetDamage();		// 캐릭터가 가지는 데미지를 반환
	float GetHPRatio();			// 현재HP와 최대MP간의 비율
};
