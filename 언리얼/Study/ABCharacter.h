// SpringArm�� ControlRoation�� ȸ�� ������ ����ǵ��� ����������, �÷��� �߿� SpringArm�� ȸ�� ���� ��ȭ�� �������ϴ�. ������ ȸ������ ������Ųä ���ο��� ���� �������� �ʰ� Control�� ȸ�� ���� �����ϴ� ������ �����˴ϴ�.
// DriectX�� ������ ���� ĳ���͸� �̵��������� ȸ����ų ��, Look���Ϳ� ī�޶��� LookȤ�� Right���� �� ������ ���� ȸ������ ���ϰ� ���� Ȥ�� ������������ ȸ���߽��ϴ�. �𸮾����� �� ������ ����� �ܼ��ϰ� ó���� �� �ִ� ����� �����ϰ� �־ ���ϴٰ� �������ϴ�.

// �� ��/�ٿ� ����
// �� �Ÿ� �Ѱ踦 ���� ��, Effect C++�� �߰��� ���鼭 Macro�� �ִ��� �����Ϸ��� static const�� ����߽��ϴ�.
// static�� const�� UPROPERTY �����ڸ� �������� �ʴµ�, �������� UPROPERTY�� �ڵ� �ʱ�ȭ ������ ������ ���� ������ �����˴ϴ�.

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
		DIABLO_MODE
	};

public:
	AABCharacter();

public:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		USpringArmComponent*			m_pSpringArm;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		UCameraComponent*				m_pCamera;

protected:
	ECONTROL_MODE		m_eControl_Mode;
	UPROPERTY(EditAnywhere, Meta = (AllowProtectedAccess))
		FVector m_FvecDirectionToMove;

protected:	
	static const float m_fZoomValue_Max;
	static const float m_fZoomValue_Min;
	static const float m_fZoomPower;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:		// Input Function
	void UpDown(float _fAxisValue);
	void LeftRight(float _fAxisValue);
	void Turn(float _fAxisValue);
	void LookUp(float _fAxisValue);
	void CameraZoom(float _fAxisValue);

private:		// Data Set Function
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SetControlMode(ECONTROL_MODE _eMode);
};
