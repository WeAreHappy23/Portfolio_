// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAnimInstance.h"

UABAnimInstance::UABAnimInstance()
{
	m_fCurrentPawnSpeed = 0.f;
	m_bInAir = false;

	static ConstructorHelpers::FObjectFinder<UAnimMontage>ATTACK_MONTAGE(TEXT("/Game/Animations/SK_Mannequin_Skeleton_Montage.SK_Mannequin_Skeleton_Montage"));
	if (true == ATTACK_MONTAGE.Succeeded())
		m_pAnimMontage = ATTACK_MONTAGE.Object;
}

void UABAnimInstance::AnimNotify_AttackHitCheck()
{
	OnAttackHitCheck.Broadcast();			// Broadcast() : �ִԸ�Ÿ�ֿ��� �ش� ��Ƽ���̰� ȣ���� �Ǹ� ��ϵ� �Լ��� ȣ���ϴ� ���
}

void UABAnimInstance::AnimNotify_NextAttackCheck()
{
	OnNextAttackCheck.Broadcast();	
}

void UABAnimInstance::PlayAttackMontage()
{
	Montage_Play(m_pAnimMontage, 1.f);
}

void UABAnimInstance::JumpToAttackMontageSection(int32 _iNewSection)
{
	ABCHECK(Montage_IsPlaying(m_pAnimMontage));
	Montage_JumpToSection(GetAttackMontageSectionName(_iNewSection), m_pAnimMontage);		// �ش� �̸��� ���� ������ �����Ѵ�.
}

FName UABAnimInstance::GetAttackMontageSectionName(int32 _iSection)
{
	ABCHECK(FMath::IsWithinInclusive<int32>(_iSection, 1, 4), NAME_None);
	return FName(*FString::Printf(TEXT("Attack%d"), _iSection));
}

void UABAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	auto Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn))
	{
		m_fCurrentPawnSpeed = Pawn->GetVelocity().Size();

		// APawn�� ���ؼ� IsFalling�Լ��� ȣ���� �� ������, Pawn�� �Լ��� ������ false�� ��ȯ�ϰ� �Ǿ��ִ�.
		// ACharacter�� ���� �����Ʈ������Ʈ���� �� ����� Ȯ���ϰ� �����ϰ� �ֱ� ������ ACharacter������ ����Ѵ�.
		auto Character = Cast<ACharacter>(Pawn);
		if (::IsValid(Character))
			m_bInAir = Character->GetMovementComponent()->IsFalling();
	}
}
