#pragma region 공부한 것
// 어떤 컴포넌트를 추가했을 때 찾을 수 없다면, 어떤 헤더에 있는지 모를 때 언리얼 문서를 참고해 추가해야함. 거기에 다 나와있음
// 언리얼엔진 프로젝트 때는 가능하면 EditAnywhere로 했던 것이 기억남. 남용하면 캡슐화의 기능을 잃게 될 수 있으므로 필요할 때만 적절히 사용해야한다고 느낌.
// URotatingMovementComponent는 EngineMinimal.h에 포함되지 않았다는 것을 알게 됨.
// UPROPERTY로 변수를 만들면 반드시 0으로 초기화 대상이 되는 것을 배움. 그래서 변수를 초기화할 때 nullptr나 값 초기화를 하지 않는 이유를 알게 됨.
// 언리얼 오브젝트가되면 자동으로 메모리관리를 받는다는 사실을 알게 됨. 
// 커스텀 매크로를 공부할 때 가변인자 매크로를 처음 접했는데, 사용자도 가변인자로 제작할 수 있다는 것을 알게 됨.
#pragma endregion

#pragma region 느낀점
// DirectX로 제작했던 방식과 유사해서 이해하기 쉬웠음
// (On_Initialize -> Update -> Late_Update -> Render -> Free 의 로직대로 제작)

// DX9에서 DX11으로 개발환경을 바꿨는데, 이때 적용한 개념이 IWYU와 비슷해서 충분히 이해할 수 있었음
// (DX9는 확장헤더를 사용했지만, DX11에서는 불필요한 기능추가를 막기위해 기본헤더에 필요한 기능을 추가하면서 사용했다.)
#pragma endregion

#pragma once

#include "ArenaBattle.h"		// 4.15버전 이후 부터 IWYU개념이 도입되어 최소의 기능을 제공하는 CoreMinimal.h이 자동 포함
#include "GameFramework/Actor.h"
#include "GameFramework/RotatingMovementComponent.h"		// URotatingMovementComponent 사용
#include "Fountain.generated.h"

UCLASS()
class ARENABATTLE_API AFountain : public AActor
{
	GENERATED_BODY()
public:	
	AFountain();

public:
	// VisibleAnywhere	모든 창에 보이지만 편집할 수 없는 상태
	// EditAnywhere		모든 창에서 편집할 수 있는 상태
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent*				m_pBody;
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent*				m_pWater;
	UPROPERTY(VisibleAnywhere)
		UPointLightComponent*					m_pLight;
	UPROPERTY(VisibleAnywhere)
		UParticleSystemComponent*			m_pSplash;
	UPROPERTY(VisibleAnywhere)
		URotatingMovementComponent*		m_pRotatingMovement;

private:
	// 책에서는 private와 EditAnywhere같이 사용해서 컴파일 에러가 발생한다고 나왔으나 직접 해본 결과 컴파일 에러는 발생하지 않았다.
	// 하지만 Meta = (AllowPrivateAccess = true) 키워드를 추가하는 것이 안전하다고 생각되어 키워드를 추가했다.
	// 여러 가지 정보를 찾아보다가 4.25버전 부터 "= true"요구 사항을 제거했다고 릴리즈 노트에 나와있는 것을 발견. (현재 버전 4.26)
	UPROPERTY(EditAnywhere, Category = "ID", Meta = (AllowPrivateAccess))		// = true 제거 테스트
		int32		m_iID;
	UPROPERTY(EditAnywhere, Category = "Stat", Meta = (AllowPrivateAccess = true))	
		float		m_fRotateSpeed;

public:
	virtual void PostInitializeComponents() override;										// 컴포넌트 초기화 후 실행 (생성자 호출 다음에 호출되었음)
	virtual void BeginPlay() override;															// 게임 플레이시 호출
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;		// 게임 종료시 호출
	virtual void Tick(float DeltaTime) override;												// 매프레임 호출. 생성자에 틱옵션을 false로 지정하면 틱함수는 호출되지 않음.
};
