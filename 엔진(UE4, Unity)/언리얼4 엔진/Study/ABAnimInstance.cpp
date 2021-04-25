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
	OnAttackHitCheck.Broadcast();			// Broadcast() : 애님몽타주에서 해당 노티파이가 호출이 되면 등록된 함수를 호출하는 기능
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
	Montage_JumpToSection(GetAttackMontageSectionName(_iNewSection), m_pAnimMontage);		// 해당 이름을 가진 섹션을 실행한다.
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

		// APawn을 통해서 IsFalling함수를 호출할 수 있지만, Pawn의 함수는 무조건 false로 반환하게 되어있다.
		// ACharacter가 가진 무브먼트컴포넌트만이 이 기능을 확실하게 구현하고 있기 때문에 ACharacter것으로 사용한다.
		auto Character = Cast<ACharacter>(Pawn);
		if (::IsValid(Character))
			m_bInAir = Character->GetMovementComponent()->IsFalling();
	}
}
