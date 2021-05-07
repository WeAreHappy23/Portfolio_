#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"

const float AABCharacter::m_fZoomValue_Max = 200.f;
const float AABCharacter::m_fZoomValue_Min = -200.f;
const float AABCharacter::m_fZoomPower = 30.f;

AABCharacter::AABCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
	m_pCharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));
	m_pHPBarUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARUI"));

	m_pSpringArm->SetupAttachment(GetCapsuleComponent());
	m_pCamera->SetupAttachment(m_pSpringArm);
	m_pHPBarUI->SetupAttachment(GetMesh());

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	m_pSpringArm->TargetArmLength = 400.f;
	m_pSpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));

	// 블루프린트 클래스가 아닌 에셋을 가져오는 것이므로 FObjectFinder를 사용한다.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SK_WARRIOR(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
	if (true == SK_WARRIOR.Succeeded())
		GetMesh()->SetSkeletalMesh(SK_WARRIOR.Object);

	// 현재 적용할 애니메이션은 단일 노드(애니메이션 하나)가 아닌 블루프린트로 가져와 적용할 것이므로 옵션을 AnimationBlueprint로 한다.
	// 블루 프린트 클래스를 가져올 때 '_C'를 마지막에 붙인다.
	// 애니메이션 블루프린트는 C++에서 UAnimInstance로 관리된다.
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FClassFinder<UAnimInstance>ANIM_WARRIOR(TEXT("/Game/Animations/WarriorAnimBluePrint.WarriorAnimBluePrint_C"));
	if (true == ANIM_WARRIOR.Succeeded())
		GetMesh()->SetAnimInstanceClass(ANIM_WARRIOR.Class);

	static ConstructorHelpers::FClassFinder<UUserWidget>UI_HUD(TEXT("/Game/UI/UI_HPBar.UI_HPBar_C"));
	if (true == UI_HUD.Succeeded())
	{
		m_pHPBarUI->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
		m_pHPBarUI->SetWidgetSpace(EWidgetSpace::Screen);
		m_pHPBarUI->SetWidgetClass(UI_HUD.Class);
		m_pHPBarUI->SetDrawSize(FVector2D(150.f, 50.f));
	}

	// View 설정
	SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);

	// Jump 설정
	GetCharacterMovement()->JumpZVelocity = 800.f;

	m_fArmLengthSpeed = 3.f;
	m_fArmRotationSpeed = 10.f;
	m_bIsAttacking = false;
	m_iMaxCombo = 4;
	AttackEndComboState();
	m_fAttackRange = 200.f;
	m_fAttackRadius = 50.f;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));
}

void AABCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	m_pAnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());		// 미리 변수로서 캐싱한다. 람다식에서도 사용 가능
	ABCHECK(nullptr != m_pAnimInstance);

	// 몽타주가 종료되면 바인딩된 함수가 호출된다.
	m_pAnimInstance->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	// 몽타주에서 해당 노티파이를 만났을 때 람다로 정의된 함수가 실행된다. 마지막 콤보는 노티파이가 없어 추가 콤보는 일어나지 않는다.
	m_pAnimInstance->OnNextAttackCheck.AddLambda([this]()->void {
		ABLOG(Warning, TEXT("OnNextAttackCheck"));
		m_bCanNextCombo = false;

		if (true == m_bInputCombo)		// 노티파이가 호출되기 전에 이 조건을 만족하면 다음 콤보 실행
		{
			AttackStartComboState();
			m_pAnimInstance->JumpToAttackMontageSection(m_iCurrentCombo);
		}
	});
	m_pAnimInstance->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck);

	m_pCharacterStat->OnHPisZeroDelegate.AddLambda([this]()->void {
		m_pAnimInstance->SetDeath();
		SetActorEnableCollision(false);
	});
}

void AABCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 4.21버전 부터 위젯의 초기화 시점이 PostInitializeComponents에서 BeginePlay로 바뀜
	// pCharacterWidget가 기능을 올바르게 수행하기 위해 BeginPlay에 작성
	auto pCharacterWidget = Cast<UABCharacterWidget>(m_pHPBarUI->GetUserWidgetObject());
	if (nullptr != pCharacterWidget)
	{
		pCharacterWidget->BindCharacterHP(m_pCharacterStat);
	}
}

void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 보간을 통해 플레이어와 카메라의 거리 설정( 스프링암 값에서 설정된 값까지 보간 )
	m_pSpringArm->TargetArmLength = FMath::FInterpTo(m_pSpringArm->TargetArmLength, m_fArmLengthTo + m_fZoom, DeltaTime, m_fArmLengthSpeed);

	if (ECONTROL_MODE::DIABLO_MODE == m_eControl_Mode)
	{
		// 보간을 통해 카메라의 각도 설정( 3인칭뷰의 각도에서 설정된 목표 각도로 보간 )
		m_pSpringArm->SetRelativeRotation(FMath::RInterpTo(m_pSpringArm->GetRelativeRotation(), m_FRotationTo, DeltaTime, m_fArmRotationSpeed));
		if (abs(m_FvecDirectionToMove.Size()) > 0.f)
		{
			// FRotationMatrix::MakeFromX -> 주어진 X축(시선방향)으로 회전값을 구하는 함수
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(m_FvecDirectionToMove).Rotator());
			AddMovementInput(m_FvecDirectionToMove);
		}
	}
}



void AABCharacter::OnAttackMontageEnded(UAnimMontage * _pMontage, bool _bInterrupted)
{
	ABCHECK(m_bIsAttacking);
	ABCHECK(0 < m_iCurrentCombo);	
	m_bIsAttacking = false;
	AttackEndComboState();
}

void AABCharacter::AttackStartComboState()
{
	m_bCanNextCombo = true;
	m_bInputCombo = false;
	ABCHECK(FMath::IsWithinInclusive<int32>(m_iCurrentCombo, 0, m_iMaxCombo - 1));		// 특정 콤보 이상이면 종료된다.
	m_iCurrentCombo = FMath::Clamp<int32>(m_iCurrentCombo + 1, 1, m_iMaxCombo);
}

void AABCharacter::AttackEndComboState()
{
	m_bCanNextCombo = false;
	m_bInputCombo = false;
	m_iCurrentCombo = 0;
}

void AABCharacter::AttackCheck()
{
	FHitResult tHitResult;
	FCollisionQueryParams tParams(NAME_None, false, this);		// 디버깅에 사용되는 이름으로 보임 / 복잡한 충돌은 하지 않는다 / this는 충돌 검사 대상에서 제외한다.
	bool	bResult = GetWorld()->SweepSingleByChannel(			// 시작과 끝 지점을 쓸면서 물리판점이 일어났는지 확인하는 함수
		tHitResult,	// 물리적 충돌이 일어날 경우 정보가 이 구조체에 담긴다.
		GetActorLocation(),	// 시작위치
		GetActorLocation() + GetActorForwardVector() * 200.f,		// 끝 위치
		FQuat::Identity,		// 탐색에 사용될 도형 회전
		ECollisionChannel::ECC_GameTraceChannel2,	// 사용할 트레이스 채널 정보 (여기서는 Attack채널)
		FCollisionShape::MakeSphere(50.f),	// 탐색에 사용할 도형
		tParams		// 위에서 정의한 바와 같이 충돌 방법에 대한 설정을 하는 구조체
	);

#if ENABLE_DRAW_DEBUG

	FVector vecTraceDir = GetActorForwardVector() * m_fAttackRange;
	FVector vecCenter = GetActorLocation() + vecTraceDir * 0.5f;
	float fHalfHeight = m_fAttackRadius + m_fAttackRange * 0.5f;
	FQuat quaCapsuleRot = FRotationMatrix::MakeFromZ(vecTraceDir).ToQuat();
	FColor colDraw = (true == bResult) ? FColor::Green : FColor::Red;
	float fDebugLifeTime = 5.f;

	DrawDebugCapsule(
		GetWorld(),
		vecCenter,
		fHalfHeight,
		m_fAttackRadius,
		quaCapsuleRot,
		colDraw,
		false,
		fDebugLifeTime
	);

#endif

	if (true == bResult)
	{
		// FHitResult의 Actor는 WeakPtr인데, 가비지콜렉션 작업을 할 수 있기 위해서다.
		// 이 객체가 다른 함수의 결과로 지워져야하는데 이 검사를 위해서 참조를 계속한다면 메모리에 계속 남아있기 때문에 이 문제를 해결하기 위해서다.
		// 이런 객체를 사용할 때 유효한지 점검해야한다.
		if (true == tHitResult.Actor.IsValid())
		{
			ABLOG(Warning, TEXT("Hit Actor Name : %s"), *tHitResult.Actor->GetName());

			FDamageEvent tDamageEvent;
			tHitResult.Actor->TakeDamage(m_pCharacterStat->GetDamage() , tDamageEvent, GetController(), this);		// 데미지를 가한 주체는 명령을 내린 컨트롤러.
		}
	}
}

void AABCharacter::UpDown(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		AddMovementInput(FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X), _fAxisValue);
	}
	else if (ECONTROL_MODE::DIABLO_MODE == m_eControl_Mode)
	{
		m_FvecDirectionToMove.X = _fAxisValue;
	}
}

void AABCharacter::LeftRight(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		AddMovementInput(FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y), _fAxisValue);
	}
	else if (ECONTROL_MODE::DIABLO_MODE == m_eControl_Mode)
	{
		m_FvecDirectionToMove.Y = _fAxisValue;
	}
}

// 로컬좌표 기준 Yaw(Up벡터) 회전
void AABCharacter::Turn(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		// 내부 동작을 보면 컨트롤러의 회전값을 설정하는 기능임을 알 수 있음.
		// SpringArm의 bUsePawnControlRotation을 true로 했기 때문에 컨트롤러의 회전값이 Spring에 적용된다.
		AddControllerYawInput(_fAxisValue);			// 컨트롤러의 Yaw축 회전 (Up벡터 축)
	}
}

// 로컬좌표 기준 Pitch(Right벡터) 회전
void AABCharacter::LookUp(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		AddControllerPitchInput(_fAxisValue);			// 컨트롤러의 Pitch축 회전  (Right벡터 축)
	}
}

// 마우스 휠로 줌설정
void AABCharacter::CameraZoom(float _fAxisValue)
{
	m_fZoom = FMath::Clamp<float>(m_fZoom + _fAxisValue * m_fZoomPower, m_fZoomValue_Min, m_fZoomValue_Max);
}

// View 모드 전환 (삼인칭 <-> 쿼터뷰)
void AABCharacter::ViewChange()
{
	// 현재 모드 상태에 따라서 다른 뷰 모드로 토글
	switch (m_eControl_Mode)
	{
	case ECONTROL_MODE::THIRD_PERSON_MODE:
		GetController()->SetControlRotation(GetActorRotation());	// 쿼터뷰에서 컨트롤러의 주요 역할은 특정 방향으로 액터가 회전할 때 컨트롤러의 회전값이 되도록 하는 것이지만 특정 방향으로 입력이 계속 되지 않으므로 이 설정은 굳이 하지 않아도 큰 문제는 없다.
		SetControlMode(ECONTROL_MODE::DIABLO_MODE);
		break;
	case ECONTROL_MODE::DIABLO_MODE:
		GetController()->SetControlRotation(m_pSpringArm->GetRelativeRotation());		// 3인칭에서 컨트롤러와 SpringArm의 회전은 대응된다. (컨트롤러의 주된 역할은 SpringArm의 회전이다.)
		SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);
		break;
	}
}

void AABCharacter::Attack()
{
	if (true == m_bIsAttacking)		// 이후 추가 콤보
	{
		ABCHECK(FMath::IsWithinInclusive<int32>(m_iCurrentCombo, 1, m_iMaxCombo));
		if (true == m_bCanNextCombo)
			m_bInputCombo = true;
	}
	else		// 첫 공격
	{
		ABCHECK(0 == m_iCurrentCombo);
		AttackStartComboState();
		m_pAnimInstance->PlayAttackMontage();
		m_pAnimInstance->JumpToAttackMontageSection(m_iCurrentCombo);
		m_bIsAttacking = true;
	}
}

// 3인칭 컨트롤 모드 설정
void AABCharacter::SetControlMode(ECONTROL_MODE _eMode)
{
	m_eControl_Mode = _eMode;

	if (ECONTROL_MODE::THIRD_PERSON_MODE == _eMode)
	{
		m_pSpringArm->bUsePawnControlRotation = true;		// SpringArm이 ControlRotation을 따르도록 한다. -> 플레이어 주위를 회전할 수 있다.
		m_pSpringArm->bInheritPitch = true;			// 회전할 때, Pitch축 회전은 무시한다.
		m_pSpringArm->bInheritYaw = true;				// 회전할 때, Yaw축 회전은 무시한다.
		m_pSpringArm->bInheritRoll = true;				// 회전할 때, Roll축 회전은 무시한다.
		m_pSpringArm->bDoCollisionTest = true;		// 플레이어<-카메라 클리핑 방지 . 변수 명이 Collision인 것으로 보아 충돌 검사를 하는 것으로 보인다.
		bUseControllerRotationYaw = false;				// true일 때, 컨트롤러의 Yaw값으로 액터의 Yaw값이 설정된다.  false일 때, 컨트롤러의 Yaw값에 액터는 영향을 받지 않는다.

		// 이 기능을 사용하면 이동 방향으로 회전할 수 있다.
		GetCharacterMovement()->bUseControllerDesiredRotation = false;		
		GetCharacterMovement()->bOrientRotationToMovement = true;			// Orient : 지향하게하다, 맞추다
		GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);		// 회전 속도

		m_fArmLengthTo = 450.f;
	}
	else if (ECONTROL_MODE::DIABLO_MODE == m_eControl_Mode)
	{
		m_pSpringArm->bUsePawnControlRotation = false;
		m_pSpringArm->bInheritPitch = false;
		m_pSpringArm->bInheritYaw = false;
		m_pSpringArm->bInheritRoll = false;
		m_pSpringArm->bDoCollisionTest = false;
		bUseControllerRotationYaw = false;	
		GetCharacterMovement()->bUseControllerDesiredRotation = true;		// 컨트롤러가 가리키는 방향으로 부드럽게 회전한다. (bUseControllerRotationYaw가 false일 때)
		GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

		m_fArmLengthTo = 800.f;
		m_FRotationTo = FRotator(-45.f, 0.f, 0.f);
	}
}

void AABCharacter::SetWeapon(AABWeapon * _pWeapon)
{
	ABCHECK(nullptr != _pWeapon && nullptr == m_pWeapon);

	FName WeaponSocketName(TEXT("hand_rSocket"));
	
	_pWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
	_pWeapon->SetOwner(this);
	m_pWeapon = _pWeapon;
}

bool AABCharacter::CanSetWeapon()
{
	return (nullptr == m_pWeapon);
}

void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("WheelUp"), this, &AABCharacter::CameraZoom);

	PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);
}

float AABCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	float fFinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ABLOG(Warning, TEXT("%s Damage : %f"), *EventInstigator->GetName(), fFinalDamage);

	m_pCharacterStat->Damaged(fFinalDamage);

	return fFinalDamage;
}