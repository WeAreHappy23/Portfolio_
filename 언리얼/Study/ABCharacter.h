// SpringArm이 ControlRoation의 회전 값으로 적용되도록 설정했지만, 플레이 중에 SpringArm의 회전 값은 변화가 없었습니다. 기존의 회전값을 유지시킨채 내부에서 값을 저장하지 않고 Control의 회전 값을 적용하는 것으로 생각됩니다.
// DriectX로 개발할 때는 캐릭터를 이동방향으로 회전시킬 때, Look벡터와 카메라의 Look혹은 Right벡터 간 외적을 통해 회전축을 구하고 내적 혹은 외적연산으로 회전했습니다. 언리얼엔진은 이 복잡한 기능을 단순하게 처리할 수 있는 방법을 제공하고 있어서 편하다고 느꼈습니다.

// 휠 업/다운 구현
// 휠 거리 한계를 정할 때, Effect C++을 추가로 보면서 Macro를 최대한 자제하려고 static const를 사용했습니다.
// static과 const는 UPROPERTY 지정자를 지원하지 않는데, 추측컨대 UPROPERTY의 자동 초기화 시점과 관련이 있을 것으로 생각됩니다.

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
