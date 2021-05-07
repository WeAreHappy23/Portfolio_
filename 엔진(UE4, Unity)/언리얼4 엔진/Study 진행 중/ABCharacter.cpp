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

	static ConstructorHelpers::FClassFinder<UUserWidget>UI_HUD(TEXT("/Game/UI/UI_HPBar.UI_HPBar_C"));
	if (true == UI_HUD.Succeeded())
	{
		m_pHPBarUI->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
		m_pHPBarUI->SetWidgetSpace(EWidgetSpace::Screen);
		m_pHPBarUI->SetWidgetClass(UI_HUD.Class);
		m_pHPBarUI->SetDrawSize(FVector2D(150.f, 50.f));
	}

	// View ����
	SetControlMode(ECONTROL_MODE::THIRD_PERSON_MODE);

	// Jump ����
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
	m_pAnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());		// �̸� �����μ� ĳ���Ѵ�. ���ٽĿ����� ��� ����
	ABCHECK(nullptr != m_pAnimInstance);

	// ��Ÿ�ְ� ����Ǹ� ���ε��� �Լ��� ȣ��ȴ�.
	m_pAnimInstance->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	// ��Ÿ�ֿ��� �ش� ��Ƽ���̸� ������ �� ���ٷ� ���ǵ� �Լ��� ����ȴ�. ������ �޺��� ��Ƽ���̰� ���� �߰� �޺��� �Ͼ�� �ʴ´�.
	m_pAnimInstance->OnNextAttackCheck.AddLambda([this]()->void {
		ABLOG(Warning, TEXT("OnNextAttackCheck"));
		m_bCanNextCombo = false;

		if (true == m_bInputCombo)		// ��Ƽ���̰� ȣ��Ǳ� ���� �� ������ �����ϸ� ���� �޺� ����
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

	// 4.21���� ���� ������ �ʱ�ȭ ������ PostInitializeComponents���� BeginePlay�� �ٲ�
	// pCharacterWidget�� ����� �ùٸ��� �����ϱ� ���� BeginPlay�� �ۼ�
	auto pCharacterWidget = Cast<UABCharacterWidget>(m_pHPBarUI->GetUserWidgetObject());
	if (nullptr != pCharacterWidget)
	{
		pCharacterWidget->BindCharacterHP(m_pCharacterStat);
	}
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
	ABCHECK(FMath::IsWithinInclusive<int32>(m_iCurrentCombo, 0, m_iMaxCombo - 1));		// Ư�� �޺� �̻��̸� ����ȴ�.
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
	FCollisionQueryParams tParams(NAME_None, false, this);		// ����뿡 ���Ǵ� �̸����� ���� / ������ �浹�� ���� �ʴ´� / this�� �浹 �˻� ��󿡼� �����Ѵ�.
	bool	bResult = GetWorld()->SweepSingleByChannel(			// ���۰� �� ������ ���鼭 ���������� �Ͼ���� Ȯ���ϴ� �Լ�
		tHitResult,	// ������ �浹�� �Ͼ ��� ������ �� ����ü�� ����.
		GetActorLocation(),	// ������ġ
		GetActorLocation() + GetActorForwardVector() * 200.f,		// �� ��ġ
		FQuat::Identity,		// Ž���� ���� ���� ȸ��
		ECollisionChannel::ECC_GameTraceChannel2,	// ����� Ʈ���̽� ä�� ���� (���⼭�� Attackä��)
		FCollisionShape::MakeSphere(50.f),	// Ž���� ����� ����
		tParams		// ������ ������ �ٿ� ���� �浹 ����� ���� ������ �ϴ� ����ü
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
		// FHitResult�� Actor�� WeakPtr�ε�, �������ݷ��� �۾��� �� �� �ֱ� ���ؼ���.
		// �� ��ü�� �ٸ� �Լ��� ����� ���������ϴµ� �� �˻縦 ���ؼ� ������ ����Ѵٸ� �޸𸮿� ��� �����ֱ� ������ �� ������ �ذ��ϱ� ���ؼ���.
		// �̷� ��ü�� ����� �� ��ȿ���� �����ؾ��Ѵ�.
		if (true == tHitResult.Actor.IsValid())
		{
			ABLOG(Warning, TEXT("Hit Actor Name : %s"), *tHitResult.Actor->GetName());

			FDamageEvent tDamageEvent;
			tHitResult.Actor->TakeDamage(m_pCharacterStat->GetDamage() , tDamageEvent, GetController(), this);		// �������� ���� ��ü�� ����� ���� ��Ʈ�ѷ�.
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
	m_fZoom = FMath::Clamp<float>(m_fZoom + _fAxisValue * m_fZoomPower, m_fZoomValue_Min, m_fZoomValue_Max);
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

void AABCharacter::Attack()
{
	if (true == m_bIsAttacking)		// ���� �߰� �޺�
	{
		ABCHECK(FMath::IsWithinInclusive<int32>(m_iCurrentCombo, 1, m_iMaxCombo));
		if (true == m_bCanNextCombo)
			m_bInputCombo = true;
	}
	else		// ù ����
	{
		ABCHECK(0 == m_iCurrentCombo);
		AttackStartComboState();
		m_pAnimInstance->PlayAttackMontage();
		m_pAnimInstance->JumpToAttackMontageSection(m_iCurrentCombo);
		m_bIsAttacking = true;
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
		GetCharacterMovement()->bOrientRotationToMovement = true;			// Orient : �����ϰ��ϴ�, ���ߴ�
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