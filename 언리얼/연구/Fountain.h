#include "ArenaBattle.h"		// 4.15���� ���� ���� IWYU������ ���ԵǾ� �ּ��� ����� �����ϴ� CoreMinimal.h�� �ڵ� ����
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
	// ������ ��Ʈ : 4.25���� ���� "= true"�䱸 ������ ����  (���� ���� 4.26)
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
