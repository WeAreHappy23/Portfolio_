#include "ABCharacter.h"

const float AABCharacter::m_fZoomValue_Max = 200.f;
const float AABCharacter::m_fZoomValue_Min = -200.f;
const float AABCharacter::m_fZoomPower = 30.f;

AABCharacter::AABCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

	m_pSpringArm->SetupAttachment(GetCapsuleComponent());
	m_pCamera->SetupAttachment(m_pSpringArm);

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
	
	// View 설정
	SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);

	m_fArmLengthSpeed = 3.f;
	m_fArmRotationSpeed = 10.f;
}

void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

void AABCharacter::UpDown(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		AddMovementInput(FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X), _fAxisValue);
		// GetControlRotation()).GetUnitAxis(EAxis::X) 방향도 있지만 컨트롤러가 가리키는 방향에 따라 속도가 달라진다. (위에서 머리를 볼 때 거의 움직이지 않는다)
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
	m_fZoom += _fAxisValue * m_fZoomPower;

	// Max와 Min 사이를 오갈 수 있도록 조건 지정
	m_fZoom = (m_fZoom > m_fZoomValue_Max) ? m_fZoomValue_Max : m_fZoom;
	m_fZoom = (m_fZoom < m_fZoomValue_Min) ? m_fZoomValue_Min : m_fZoom;
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
		GetCharacterMovement()->bOrientRotationToMovement = true;
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

void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);

	PlayerInputComponent->BindAxis(TEXT("WheelUp"), this, &AABCharacter::CameraZoom);

	PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
}

