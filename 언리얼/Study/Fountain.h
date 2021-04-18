#pragma region ������ ��
// � ������Ʈ�� �߰����� �� ã�� �� ���ٸ�, � ����� �ִ��� �� �� �𸮾� ������ ������ �߰��ؾ���. �ű⿡ �� ��������
// �𸮾��� ������Ʈ ���� �����ϸ� EditAnywhere�� �ߴ� ���� ��ﳲ. �����ϸ� ĸ��ȭ�� ����� �Ұ� �� �� �����Ƿ� �ʿ��� ���� ������ ����ؾ��Ѵٰ� ����.
// URotatingMovementComponent�� EngineMinimal.h�� ���Ե��� �ʾҴٴ� ���� �˰� ��.
// UPROPERTY�� ������ ����� �ݵ�� 0���� �ʱ�ȭ ����� �Ǵ� ���� ���. �׷��� ������ �ʱ�ȭ�� �� nullptr�� �� �ʱ�ȭ�� ���� �ʴ� ������ �˰� ��.
// �𸮾� ������Ʈ���Ǹ� �ڵ����� �޸𸮰����� �޴´ٴ� ����� �˰� ��. 
// Ŀ���� ��ũ�θ� ������ �� �������� ��ũ�θ� ó�� ���ߴµ�, ����ڵ� �������ڷ� ������ �� �ִٴ� ���� �˰� ��.
#pragma endregion

#pragma region ������
// DirectX�� �����ߴ� ��İ� �����ؼ� �����ϱ� ������
// (On_Initialize -> Update -> Late_Update -> Render -> Free �� ������� ����)

// DX9���� DX11���� ����ȯ���� �ٲ�µ�, �̶� ������ ������ IWYU�� ����ؼ� ����� ������ �� �־���
// (DX9�� Ȯ������� ���������, DX11������ ���ʿ��� ����߰��� �������� �⺻����� �ʿ��� ����� �߰��ϸ鼭 ����ߴ�.)
#pragma endregion

#pragma once

#include "ArenaBattle.h"		// 4.15���� ���� ���� IWYU������ ���ԵǾ� �ּ��� ����� �����ϴ� CoreMinimal.h�� �ڵ� ����
#include "GameFramework/Actor.h"
#include "GameFramework/RotatingMovementComponent.h"		// URotatingMovementComponent ���
#include "Fountain.generated.h"

UCLASS()
class ARENABATTLE_API AFountain : public AActor
{
	GENERATED_BODY()
public:	
	AFountain();

public:
	// VisibleAnywhere	��� â�� �������� ������ �� ���� ����
	// EditAnywhere		��� â���� ������ �� �ִ� ����
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
	// å������ private�� EditAnywhere���� ����ؼ� ������ ������ �߻��Ѵٰ� �������� ���� �غ� ��� ������ ������ �߻����� �ʾҴ�.
	// ������ Meta = (AllowPrivateAccess = true) Ű���带 �߰��ϴ� ���� �����ϴٰ� �����Ǿ� Ű���带 �߰��ߴ�.
	// ���� ���� ������ ã�ƺ��ٰ� 4.25���� ���� "= true"�䱸 ������ �����ߴٰ� ������ ��Ʈ�� �����ִ� ���� �߰�. (���� ���� 4.26)
	UPROPERTY(EditAnywhere, Category = "ID", Meta = (AllowPrivateAccess))		// = true ���� �׽�Ʈ
		int32		m_iID;
	UPROPERTY(EditAnywhere, Category = "Stat", Meta = (AllowPrivateAccess = true))	
		float		m_fRotateSpeed;

public:
	virtual void PostInitializeComponents() override;										// ������Ʈ �ʱ�ȭ �� ���� (������ ȣ�� ������ ȣ��Ǿ���)
	virtual void BeginPlay() override;															// ���� �÷��̽� ȣ��
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;		// ���� ����� ȣ��
	virtual void Tick(float DeltaTime) override;												// �������� ȣ��. �����ڿ� ƽ�ɼ��� false�� �����ϸ� ƽ�Լ��� ȣ����� ����.
};
