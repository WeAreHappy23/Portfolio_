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
	// FRotationMatrix은 FRotator를 인자로 받아서 내부에서 삼각함수요소로 변환해 행렬을 만든다. 이 때, 마지막 위치 벡터는 ZeroVector다.
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), _fAxisValue);	// X(Roll, Look벡터)방향으로 _fAxisValue에 따라 움직인다.
}

void AABCharacter::LeftRight(float _fAxisValue)
{
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), _fAxisValue);		// Y(Pitch, Right벡터)방향으로 _fAxisValue에 따라 움직인다.
}

void AABCharacter::Turn(float _fAxisValue)
{
	// 내부 동작을 보면 컨트롤러의 회전값을 설정하는 기능임을 알 수 있음.
	// SpringArm의 bUsePawnControlRotation을 true로 했기 때문에 컨트롤러의 회전값이 Spring에 적용된다.
	AddControllerYawInput(_fAxisValue);			// 컨트롤러의 Yaw축 회전 (Up벡터 축)
}

void AABCharacter::LookUp(float _fAxisValue)
{
	AddControllerPitchInput(_fAxisValue);			// 컨트롤러의 Pitch축 회전  (Right벡터 축)
}

void AABCharacter::CameraZoom(float _fAxisValue)
{
	m_pSpringArm->TargetArmLength += _fAxisValue * m_fZoomPower;

	m_pSpringArm->TargetArmLength = (m_pSpringArm->TargetArmLength > m_fZoomValue_Max) ? m_fZoomValue_Max : m_pSpringArm->TargetArmLength;
	m_pSpringArm->TargetArmLength = (m_pSpringArm->TargetArmLength < m_fZoomValue_Min) ? m_fZoomValue_Min : m_pSpringArm->TargetArmLength;
}

// 3인칭 컨트롤
void AABCharacter::SetControlMode(ECONTROL_MODE _eMode)
{
	if (ECONTROL_MODE::THIRD_PERSON_MODE == _eMode)
	{
		m_pSpringArm->TargetArmLength = 450.f;				// 타겟과의 거리를 설정한다.
		m_pSpringArm->SetRelativeRotation(FRotator::ZeroRotator);		// 부모 컴포넌트(여기서는 캡슐컴포넌트)와의 회전을  Zero로 설정한다.
		m_pSpringArm->bUsePawnControlRotation = true;		// SpringArm이 ControlRotation을 따르도록 한다. -> 플레이어 주위를 회전할 수 있다.
		m_pSpringArm->bInheritPitch = true;			// 회전할 때, Pitch축 회전은 무시한다.
		m_pSpringArm->bInheritYaw = true;				// 회전할 때, Yaw축 회전은 무시한다.
		m_pSpringArm->bInheritRoll = true;				// 회전할 때, Roll축 회전은 무시한다.
		m_pSpringArm->bDoCollisionTest = true;		// 반드시 플레이어가 보이도록 해준다. 변수 명이 Collision인 것으로 보아 충돌 검사를 하는 것으로 보인다.
		bUseControllerRotationYaw = false;				// 액터가 컨트롤러의 Yaw값을 따르지 않도록한다. -> true면, 컨트롤러의 Yaw와 동일시되서 뒷 모습만 보인다.

		// 이 기능을 사용하면 이동 방향으로 회전할 수 있다.
		GetCharacterMovement()->bOrientRotationToMovement = true;			// Orient : 지향하게하다, 맞추다
		GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);		// 회전 속도
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

