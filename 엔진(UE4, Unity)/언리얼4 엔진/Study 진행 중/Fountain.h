#include "ArenaBattle.h"		// 4.15버전 이후 부터 IWYU개념이 도입되어 최소의 기능을 제공하는 CoreMinimal.h이 자동 포함
#include "GameFramework/Actor.h"
#include "GameFramework/RotatingMovementComponent.h"	
#include "Fountain.generated.h"

UCLASS()
class ARENABATTLE_API AFountain : public AActor
{
	GENERATED_BODY()
public:	
	AFountain();

public:
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
	// 릴리즈 노트 : 4.25버전 부터 "= true"요구 사항을 제거  (현재 버전 4.26)
	UPROPERTY(EditAnywhere, Category = "ID", Meta = (AllowPrivateAccess))	
		int32		m_iID;
	UPROPERTY(EditAnywhere, Category = "Stat", Meta = (AllowPrivateAccess = true))	
		float		m_fRotateSpeed;

public:
	virtual void PostInitializeComponents() override;	
	virtual void BeginPlay() override;	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
	virtual void Tick(float DeltaTime) override;	
};
