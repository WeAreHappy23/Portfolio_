#include "ABCharacter.h"

const float AABCharacter::m_fZoomValue_Max = 600.f;
const float AABCharacter::m_fZoomValue_Min = 300.f;
const float AABCharacter::m_fZoomPower = 25.f;

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

	SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);
}

void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AABCharacter::UpDown(float _fAxisValue)
{
	// FRotationMatrix�� FRotator�� ���ڷ� �޾Ƽ� ���ο��� �ﰢ�Լ���ҷ� ��ȯ�� ����� �����. �� ��, ������ ��ġ ���ʹ� ZeroVector��.
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), _fAxisValue);	// X(Roll, Look����)�������� _fAxisValue�� ���� �����δ�.
}

void AABCharacter::LeftRight(float _fAxisValue)
{
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), _fAxisValue);		// Y(Pitch, Right����)�������� _fAxisValue�� ���� �����δ�.
}

void AABCharacter::Turn(float _fAxisValue)
{
	// ���� ������ ���� ��Ʈ�ѷ��� ȸ������ �����ϴ� ������� �� �� ����.
	// SpringArm�� bUsePawnControlRotation�� true�� �߱� ������ ��Ʈ�ѷ��� ȸ������ Spring�� ����ȴ�.
	AddControllerYawInput(_fAxisValue);			// ��Ʈ�ѷ��� Yaw�� ȸ�� (Up���� ��)
}

void AABCharacter::LookUp(float _fAxisValue)
{
	AddControllerPitchInput(_fAxisValue);			// ��Ʈ�ѷ��� Pitch�� ȸ��  (Right���� ��)
}

void AABCharacter::CameraZoom(float _fAxisValue)
{
	m_pSpringArm->TargetArmLength += _fAxisValue * m_fZoomPower;

	m_pSpringArm->TargetArmLength = (m_pSpringArm->TargetArmLength > m_fZoomValue_Max) ? m_fZoomValue_Max : m_pSpringArm->TargetArmLength;
	m_pSpringArm->TargetArmLength = (m_pSpringArm->TargetArmLength < m_fZoomValue_Min) ? m_fZoomValue_Min : m_pSpringArm->TargetArmLength;
}

// 3��Ī ��Ʈ��
void AABCharacter::SetControlMode(ECONTROL_MODE _eMode)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == _eMode)
	{
		m_pSpringArm->TargetArmLength = 450.f;				// Ÿ�ٰ��� �Ÿ��� �����Ѵ�.
		m_pSpringArm->SetRelativeRotation(FRotator::ZeroRotator);		// �θ� ������Ʈ(���⼭�� ĸ��������Ʈ)���� ȸ����  Zero�� �����Ѵ�.
		m_pSpringArm->bUsePawnControlRotation = true;		// SpringArm�� ControlRotation�� �������� �Ѵ�. -> �÷��̾� ������ ȸ���� �� �ִ�.
		m_pSpringArm->bInheritPitch = true;			// ȸ���� ��, Pitch�� ȸ���� �����Ѵ�.
		m_pSpringArm->bInheritYaw = true;				// ȸ���� ��, Yaw�� ȸ���� �����Ѵ�.
		m_pSpringArm->bInheritRoll = true;				// ȸ���� ��, Roll�� ȸ���� �����Ѵ�.
		m_pSpringArm->bDoCollisionTest = true;		// �ݵ�� �÷��̾ ���̵��� ���ش�. ���� ���� Collision�� ������ ���� �浹 �˻縦 �ϴ� ������ ���δ�.
		bUseControllerRotationYaw = false;				// ���Ͱ� ��Ʈ�ѷ��� Yaw���� ������ �ʵ����Ѵ�. -> true��, ��Ʈ�ѷ��� Yaw�� ���ϽõǼ� �� ����� ���δ�.

		// �� ����� ����ϸ� �̵� �������� ȸ���� �� �ִ�.
		GetCharacterMovement()->bOrientRotationToMovement = true;			// Orient : �����ϰ��ϴ�, ���ߴ�
		GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);		// ȸ�� �ӵ�
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
}

