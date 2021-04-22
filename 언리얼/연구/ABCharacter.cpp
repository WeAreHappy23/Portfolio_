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

	// �������Ʈ Ŭ������ �ƴ� ������ �������� ���̹Ƿ� FObjectFinder�� ����Ѵ�.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SK_WARRIOR(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
	if (true == SK_WARRIOR.Succeeded())
		GetMesh()->SetSkeletalMesh(SK_WARRIOR.Object);

	// ���� ������ �ִϸ��̼��� ���� ���(�ִϸ��̼� �ϳ�)�� �ƴ� �������Ʈ�� ������ ������ ���̹Ƿ� �ɼ��� AnimationBlueprint�� �Ѵ�.
	// ��� ����Ʈ Ŭ������ ������ �� '_C'�� �������� ���δ�.
	// �ִϸ��̼� �������Ʈ�� C++���� UAnimInstance�� �����ȴ�.
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FClassFinder<UAnimInstance>ANIM_WARRIOR(TEXT("/Game/Animations/WarriorAnimBluePrint.WarriorAnimBluePrint_C"));
	if (true == ANIM_WARRIOR.Succeeded())
		GetMesh()->SetAnimInstanceClass(ANIM_WARRIOR.Class);
	
	// View ����
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

	// ������ ���� �÷��̾�� ī�޶��� �Ÿ� ����( �������� ������ ������ ������ ���� )
	m_pSpringArm->TargetArmLength = FMath::FInterpTo(m_pSpringArm->TargetArmLength, m_fArmLengthTo + m_fZoom, DeltaTime, m_fArmLengthSpeed);

	if (ECONTROL_MODE::DIABLO_MODE == m_eControl_Mode)
	{
		// ������ ���� ī�޶��� ���� ����( 3��Ī���� �������� ������ ��ǥ ������ ���� )
		m_pSpringArm->SetRelativeRotation(FMath::RInterpTo(m_pSpringArm->GetRelativeRotation(), m_FRotationTo, DeltaTime, m_fArmRotationSpeed));
		if (abs(m_FvecDirectionToMove.Size()) > 0.f)
		{
			// FRotationMatrix::MakeFromX -> �־��� X��(�ü�����)���� ȸ������ ���ϴ� �Լ�
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
		// GetControlRotation()).GetUnitAxis(EAxis::X) ���⵵ ������ ��Ʈ�ѷ��� ����Ű�� ���⿡ ���� �ӵ��� �޶�����. (������ �Ӹ��� �� �� ���� �������� �ʴ´�)
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

// ������ǥ ���� Yaw(Up����) ȸ��
void AABCharacter::Turn(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		// ���� ������ ���� ��Ʈ�ѷ��� ȸ������ �����ϴ� ������� �� �� ����.
		// SpringArm�� bUsePawnControlRotation�� true�� �߱� ������ ��Ʈ�ѷ��� ȸ������ Spring�� ����ȴ�.
		AddControllerYawInput(_fAxisValue);			// ��Ʈ�ѷ��� Yaw�� ȸ�� (Up���� ��)
	}
}

// ������ǥ ���� Pitch(Right����) ȸ��
void AABCharacter::LookUp(float _fAxisValue)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == m_eControl_Mode)
	{
		AddControllerPitchInput(_fAxisValue);			// ��Ʈ�ѷ��� Pitch�� ȸ��  (Right���� ��)
	}
}

// ���콺 �ٷ� �ܼ���
void AABCharacter::CameraZoom(float _fAxisValue)
{
	m_fZoom += _fAxisValue * m_fZoomPower;

	// Max�� Min ���̸� ���� �� �ֵ��� ���� ����
	m_fZoom = (m_fZoom > m_fZoomValue_Max) ? m_fZoomValue_Max : m_fZoom;
	m_fZoom = (m_fZoom < m_fZoomValue_Min) ? m_fZoomValue_Min : m_fZoom;
}

// View ��� ��ȯ (����Ī <-> ���ͺ�)
void AABCharacter::ViewChange()
{
	// ���� ��� ���¿� ���� �ٸ� �� ���� ���
	switch (m_eControl_Mode)
	{
	case ECONTROL_MODE::THIRD_PERSON_MODE:
		GetController()->SetControlRotation(GetActorRotation());	// ���ͺ信�� ��Ʈ�ѷ��� �ֿ� ������ Ư�� �������� ���Ͱ� ȸ���� �� ��Ʈ�ѷ��� ȸ������ �ǵ��� �ϴ� �������� Ư�� �������� �Է��� ��� ���� �����Ƿ� �� ������ ���� ���� �ʾƵ� ū ������ ����.
		SetControlMode(ECONTROL_MODE::DIABLO_MODE);
		break;
	case ECONTROL_MODE::DIABLO_MODE:
		GetController()->SetControlRotation(m_pSpringArm->GetRelativeRotation());		// 3��Ī���� ��Ʈ�ѷ��� SpringArm�� ȸ���� �����ȴ�. (��Ʈ�ѷ��� �ֵ� ������ SpringArm�� ȸ���̴�.)
		SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);
		break;
	}
}

// 3��Ī ��Ʈ�� ��� ����
void AABCharacter::SetControlMode(ECONTROL_MODE _eMode)
{
	m_eControl_Mode = _eMode;

	if (ECONTROL_MODE::THIRD_PERSON_MODE == _eMode)
	{
		m_pSpringArm->bUsePawnControlRotation = true;		// SpringArm�� ControlRotation�� �������� �Ѵ�. -> �÷��̾� ������ ȸ���� �� �ִ�.
		m_pSpringArm->bInheritPitch = true;			// ȸ���� ��, Pitch�� ȸ���� �����Ѵ�.
		m_pSpringArm->bInheritYaw = true;				// ȸ���� ��, Yaw�� ȸ���� �����Ѵ�.
		m_pSpringArm->bInheritRoll = true;				// ȸ���� ��, Roll�� ȸ���� �����Ѵ�.
		m_pSpringArm->bDoCollisionTest = true;		// �÷��̾�<-ī�޶� Ŭ���� ���� . ���� ���� Collision�� ������ ���� �浹 �˻縦 �ϴ� ������ ���δ�.
		bUseControllerRotationYaw = false;				// true�� ��, ��Ʈ�ѷ��� Yaw������ ������ Yaw���� �����ȴ�.  false�� ��, ��Ʈ�ѷ��� Yaw���� ���ʹ� ������ ���� �ʴ´�.

		// �� ����� ����ϸ� �̵� �������� ȸ���� �� �ִ�.
		GetCharacterMovement()->bUseControllerDesiredRotation = false;		
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);		// ȸ�� �ӵ�

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
		GetCharacterMovement()->bUseControllerDesiredRotation = true;		// ��Ʈ�ѷ��� ����Ű�� �������� �ε巴�� ȸ���Ѵ�. (bUseControllerRotationYaw�� false�� ��)
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

