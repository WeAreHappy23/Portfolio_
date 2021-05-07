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
	// Transient Ű����
	//  UObject���� ����ȭ ����� �־� UPROPERTY�Ӽ��� ���̺�/�ε��� �� �ִ�. ������ CurrentHP�� ����
	// ������ ������ �� ���� ����Ǵ� ���� �����ϴ� ���� �ǹ̰� ���� ���ʿ��� ��ũ ������ �����Ѵ�. Transient�� ����ȭ ������� ���� �ʰڴٴ� Ű����

	struct FABCharacterData*	m_ptStatData = nullptr;		// ��������� �߰����� �ʱ� ���ؼ� classó�� �Ｎ���� struct ���漱�����ϰ� �����͸� ���
	UPROPERTY(EditInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess))
		int32			m_iLevel;
	UPROPERTY(Transient, VisibleInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess))
		float			m_fCurrentHP;

public:
	FOnHPisZeroDelegate		OnHPisZeroDelegate;
	FOnHPChangedDelegate		OnHPChangedDelegate;

protected:
	virtual void InitializeComponent() override;		// ������ PostInitializeComponents�� �����ϴ� �Լ�. PostInitializeComponents�Լ����� ���� ȣ��
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetNewLevel(int32 _iLevel);	// ���� ����
	void Damaged(float _fAttack);		// �Ű������� ������ �������� �޴� �Լ�
	void SetHP(float _fHP);	// �Ű������� ������ HP�� ����
	float GetDamage();		// ĳ���Ͱ� ������ �������� ��ȯ
	float GetHPRatio();			// ����HP�� �ִ�MP���� ����
};
