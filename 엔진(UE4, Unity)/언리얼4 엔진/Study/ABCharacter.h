#pragma once

#include "ArenaBattle.h"
#include "GameFramework/Character.h"
#include "Arena_Enum.h"

#include "ABCharacter.generated.h"


UCLASS()
class ARENABATTLE_API AABCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	enum class ECONTROL_MODE 
	{
		THIRD_PERSON_MODE,
		DIABLO_MODE,
		CONTROL_MODE_END
	};

public:
	AABCharacter();

public:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		USpringArmComponent*			m_pSpringArm;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		UCameraComponent*				m_pCamera;

protected:
	ECONTROL_MODE		m_eControl_Mode = ECONTROL_MODE::CONTROL_MODE_END;
	FVector						m_FvecDirectionToMove = FVector::ZeroVector;

	float		m_fZoom = 0.f;

	float		m_fArmLengthTo = 0.f;
	float		m_fArmLengthSpeed = 0.f;
	float		m_fArmRotationSpeed = 0.f;
	FRotator m_FRotationTo = FRotator::ZeroRotator;

	// Animation
	UPROPERTY()
	class UABAnimInstance*		m_pAnimInstance;

	// Attack
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess))
		bool		m_bIsAttacking;			// �ִ� ��Ÿ�� ��������Ʈ�� ���� ���� ���θ� Ȯ��. ���ͷ�Ʈ ������� ó�� ��
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess))
		bool		m_bCanNextCombo;		// �޺� Ÿ�̹��� ��ġ�� �� �̻� �޺��� ���� ���ϰ� �ϴ� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess))
		bool		m_bInputCombo;			// �޺� �Է��� �־������� ���� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess))
		int32		m_iCurrentCombo;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess))
		int32		m_iMaxCombo;

protected:	
	static const float m_fZoomValue_Max;
	static const float m_fZoomValue_Min;
	static const float m_fZoomPower;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

private:		// ��������Ʈ
	UFUNCTION()
		void OnAttackMontageEnded(UAnimMontage* _pMontage, bool _bInterrupted);

private:
	UFUNCTION()
		void AttackStartComboState();
	UFUNCTION()
		void AttackEndComboState();

private:		// Input Function
	// Axis
	void UpDown(float _fAxisValue);
	void LeftRight(float _fAxisValue);
	void Turn(float _fAxisValue);
	void LookUp(float _fAxisValue);
	void CameraZoom(float _fAxisValue);

	// Action
	void ViewChange();
	void Attack();

private:		// Data Set Function
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SetControlMode(ECONTROL_MODE _eMode);
};
