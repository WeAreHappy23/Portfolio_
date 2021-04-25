#include "MotionControllerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "HandMotionController/LeftHandMotionController.h"
#include "HandMotionController/RightHandMotionController.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

#include "Public/TimerManager.h" 
#include "Equipment/PlayerShield.h"
#include "Components/WidgetComponent.h"
#include "HandMotionController/Widget/LeftHandWidget.h"

#include "Components/StereoLayerComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#include "MyCharacter/Widget/HitBloodyWidget.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

#include "Equipment/PlayerSword.h"

#include "Monster/Dog/Dog.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MyCharacter/Widget/HPStaminaBar.h"

#include "MyCharacter/Widget/Menu.h"
#include "Components/WidgetInteractionComponent.h"	
#include "TimerManager.h"
#include "Components/PawnNoiseEmitterComponent.h"

// 캐릭터입니다. 주석 부분은 다른 프로그래머가 작성습니다.

AMotionControllerCharacter::AMotionControllerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;
	SpringArm->TargetArmLength = 1.0f;

	GetCapsuleComponent()->bHiddenInGame = false;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
	HeadBox->SetupAttachment(Camera);

	// 카메라 앞에 Stereo Layer Component를 붙이고, 위젯을 나타내도록 했습니다. (피격 시 붉은 화면 UI)
	Stereo = CreateDefaultSubobject<UStereoLayerComponent>(TEXT("StereoB"));
	Stereo->SetupAttachment(Camera);

	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetB"));
	Widget->SetupAttachment(Camera);

	Stereo->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
	Stereo->bLiveTexture = true;
	Stereo->SetQuadSize(FVector2D(250.0f, 250.0f));

	// 피격 시 발생하는 위젯을 찾아서 적용합니다.
	static ConstructorHelpers::FClassFinder<UUserWidget> HitUI(TEXT("WidgetBlueprint'/Game/Blueprints/UI/BloodEffectHUD.BloodEffectHUD_C'"));
	if (HitUI.Succeeded())
	{
		Widget->SetWidgetClass(HitUI.Class);
	}

	Widget->SetWidgetSpace(EWidgetSpace::World);
	Widget->SetDrawSize(FVector2D(1000.0f, 1000.0f));
	Widget->bVisible = true;

	// 머리 콜리전의 크기를 설정하고 콜리전 작용 조건을 설정합니다.
	HeadBox->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	HeadBox->SetCollisionProfileName(TEXT("OverlapAll"));
	HeadBox->bGenerateOverlapEvents = true;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 나머지 변수들을 초기화합니다.
	MaxHp = 100.0f;
	CurrentHp = MaxHp;
	MaxStamina = 100.0f;
	CurrentStamina = MaxStamina;

	AttackPoint = 5.0f;
	DefencePoint = 10.0f;
	DashPoint = 30.0f;
	RecoveryPoint = 1.0f;
	bIsUseStamina = false;

	InvincibleTimeOn = false;
	CurrentState = EPlayerState::Idle;
	bAllowBreathe = true;
	DashPower = 800.0f;
	GrabState = E_HandState::Open;

	HeadBox->ComponentTags.Add(FName(TEXT("DisregardForLeftHand")));
	HeadBox->ComponentTags.Add(FName(TEXT("DisregardForRightHand")));
	HeadBox->ComponentTags.Add(FName("Head"));
	Tags.Add(FName("Character"));
	Tags.Add(FName(TEXT("DisregardForLeftHand")));
	Tags.Add(FName(TEXT("DisregardForRightHand")));
}

void AMotionControllerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//FName DeviceName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName();

	//if (DeviceName == "SteamVR" || DeviceName == "OculusHMD")
	//{
	//	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	//}

	//FActorSpawnParameters SpawnActorOption;
	//SpawnActorOption.Owner = this;
	//SpawnActorOption.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	//LeftHand = GetWorld()->SpawnActor<ALeftHandMotionController>(LeftHand->StaticClass(), GetMesh()->GetComponentLocation(), GetMesh()->GetComponentRotation(), SpawnActorOption);
	//if (LeftHand)
	//	LeftHand->AttachToComponent(GetMesh(), AttachRules);

	//RightHand = GetWorld()->SpawnActor<ARightHandMotionController>(RightHand->StaticClass(), GetMesh()->GetComponentLocation(), GetMesh()->GetComponentRotation(), SpawnActorOption);
	//if (RightHand)
	//	RightHand->AttachToComponent(GetMesh(), AttachRules);

	// 머리 오버랩 이벤트 설정
	if (HeadBox)
		HeadBox->OnComponentBeginOverlap.AddDynamic(this, &AMotionControllerCharacter::OnHeadOverlap);

	// 자동적으로 스태미너를 채워주도록했습니다.
	GetWorld()->GetTimerManager().SetTimer(AutoTimerHandle, this, &AMotionControllerCharacter::AutoStamina, 0.05f, false);
}

void AMotionControllerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentHp > 100.0f)
	{
		CurrentHp = 100.0f;
	}
	if (CurrentStamina > 100.0f)
	{
		CurrentStamina = 100.0f;
	}

	// 몬스터가 자신을 발견하면 배열에 딱 저장할 크기만큼 할당했습니다.
	if (DogArray.Num() > 0)
	{
		DogArray.Shrink();		// 최적화
	}

	// Sin 함수로 통해 숨쉬는 효과를 주었습니다.
	if (SpringArm)
	{
		if (bAllowBreathe)
		{
			FVector newLocation = SpringArm->RelativeLocation;
			float loca = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
			newLocation.Z += loca * 10.f;
			RunningTime += (DeltaTime * 5);

			SpringArm->SetRelativeLocation(newLocation);
		}
	}

	if (IsValid(Widget->GetRenderTarget()))
	{
		UTexture* texture;
		texture = Cast<UTextureRenderTarget2D>(Widget->GetRenderTarget());
		Stereo->SetTexture(Widget->GetRenderTarget());
	}

	// 이동시 숨을 멈춥니다.
	if (CurrentState != EPlayerState::Idle && this->GetVelocity().Size() == 0)
	{
		CurrentState = EPlayerState::Idle;
		GetWorld()->GetTimerManager().SetTimer(SetIdleTimerHandle, this, &AMotionControllerCharacter::SetAllowBreathe, 0.01f, false, 1.0f);
	}
	else if (this->GetVelocity().Size() > 0)
	{
		bAllowBreathe = false;
	}
}

// Called to bind functionality to input
void AMotionControllerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Pressed, this, &AMotionControllerCharacter::GrabLeftOn);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Released, this, &AMotionControllerCharacter::GrabLeftOff);

	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Pressed, this, &AMotionControllerCharacter::GrabRightOn);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Released, this, &AMotionControllerCharacter::GrabRightOff);

	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AMotionControllerCharacter::DashOn);
	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Released, this, &AMotionControllerCharacter::DashOff);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMotionControllerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMotionControllerCharacter::MoveRight);

	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &AMotionControllerCharacter::RunOn);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &AMotionControllerCharacter::RunOff);
	//  Test
	PlayerInputComponent->BindAction(TEXT("Menu"), IE_Released, this, &AMotionControllerCharacter::GameMenu);
}

// 왼손 그랩
void AMotionControllerCharacter::GrabLeftOn()
{
	LeftHand->interaction->PressPointerKey(EKeys::LeftMouseButton);
	GrabState = E_HandState::Grab;
	LeftHand->GrabActor();
	LeftHand->Shield->ConvertOfOpacity(0.14f);		// 방패의 투명도를 낮춥니다
}

// 왼손 그랩 해제
void AMotionControllerCharacter::GrabLeftOff()
{
	LeftHand->interaction->ReleasePointerKey(EKeys::LeftMouseButton);
	GrabState = E_HandState::Open;
	LeftHand->ReleaseActor();
	LeftHand->Shield->ConvertOfOpacity(0.8f);		// 방패의 투명도를 높입니다
}

// 오른손 그랩
void AMotionControllerCharacter::GrabRightOn()
{
	RightHand->interaction->PressPointerKey(EKeys::LeftMouseButton);
	GrabState = E_HandState::Grab;
	RightHand->GrabActor();
	RightHand->Sword->ConvertOfOpacity(1);			// 검의 투명도를 낮춥니다
}

// 오른손 그랩 해제
void AMotionControllerCharacter::GrabRightOff()
{
	RightHand->interaction->ReleasePointerKey(EKeys::LeftMouseButton);
	GrabState = E_HandState::Open;
	RightHand->ReleaseActor();
	RightHand->Sword->ConvertOfOpacity(0.5f);		// 검의 투명도를 높입니다
}

// 플레이어의 상태에 따라서 걷기/달리기 설정
void AMotionControllerCharacter::MoveForward(float Value)
{
	if (Value != 0)
	{
		if (CurrentState == EPlayerState::Run)
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		else
		{
			CurrentState = EPlayerState::Walk;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		AddMovementInput(Camera->GetForwardVector(), Value);
	}
}

// 플레이어의 상태에 따라서 걷기/달리기 설정
void AMotionControllerCharacter::MoveRight(float Value)
{
	if (Value != 0)
	{
		if (CurrentState == EPlayerState::Run)
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		else
		{
			CurrentState = EPlayerState::Walk;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		AddMovementInput(Camera->GetRightVector(), Value);
	}
}

void AMotionControllerCharacter::RunOn()
{
	CurrentState = EPlayerState::Run;
}

void AMotionControllerCharacter::RunOff()
{
	CurrentState = EPlayerState::Idle;
}

// 대쉬
void AMotionControllerCharacter::DashOn()
{
	if (CurrentStamina > DashPoint)
	{
		if (GetVelocity().Size() >= 20.0f)
		{
			UseStamina(DashPoint);
			FVector DashVector = FVector::ZeroVector;
			GetCharacterMovement()->GroundFriction = 0;					// 땅과의 마찰을 무시합니다.
			DashVector = GetVelocity().GetSafeNormal() * 3000.0f;
			DashVector.Z = 0;
			LaunchCharacter(DashVector, false, false);
		}
	}
}

// 대쉬 해제시 마찰을 원상복구시킵니다.
void AMotionControllerCharacter::DashOff()
{
	GetCharacterMovement()->GroundFriction = 8.0f;
}

// 컨트롤러 메뉴를 누르면 캐릭터 정면에 메뉴창이 생성됩니다.
void AMotionControllerCharacter::GameMenu()
{
	// 메뉴 생성
	if (!Menu)
	{
		FActorSpawnParameters SpawnActorOption;
		SpawnActorOption.Owner = this;
		SpawnActorOption.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 메뉴를 플레이어가 보는 위치와 각도로 생성합니다.
		FVector location = FVector(Camera->GetComponentLocation().X, Camera->GetComponentLocation().Y, GetActorLocation().Z);
		FRotator rotator = FRotator(Camera->GetComponentRotation().Pitch + 20.0f, Camera->GetComponentRotation().Yaw + 180.0f, 0.0f);
		FVector CameraForwardVectorzeroHeight = FVector(Camera->GetForwardVector().X, Camera->GetForwardVector().Y, 0.0f);

		Menu = GetWorld()->SpawnActor<AMenu>(Menu->StaticClass(), location + CameraForwardVectorzeroHeight.GetSafeNormal() * 70.0f,
			rotator, SpawnActorOption);
	}
	//	다시 누르면 사라집니다.
	else
	{
		Menu->Destroy();
		Menu = nullptr;
	}
}

// 타이머를 통해 조금 후에 다시 숨쉴 수 있도록 구현했습니다.
void AMotionControllerCharacter::SetAllowBreathe()
{
	bAllowBreathe = true;
}

// 데미지를 받습니다.
float AMotionControllerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!InvincibleTimeOn)
	{
		bisHit = true;
		Widget->bVisible = true;

		if (Widget->bVisible)
		{
			UHitBloodyWidget* bloodyWidget = Cast<UHitBloodyWidget>(Widget->GetUserWidgetObject());
			if (bloodyWidget)
			{
				bloodyWidget->PlayAnimationByName("Bloody", 0.0, 1, EUMGSequencePlayMode::Forward, 1.0f);		// UI가 빨간색으로 깜빡이는 애니메이션을 실행합니다.
			}
		}

		// 체력 감서
		if (CurrentHp > 0.0f)
		{
			CurrentHp -= Damage;
			if (CurrentHp < 0.0f)
				CurrentHp = 0.0f;
		}

		LeftHand->Shield->StateBar->GetDamage(Damage);
		GLog->Log(FString::Printf(TEXT("데미지 받음")));
		InvincibleTimeOn = true;		// 지속적으로 피해를 입지 않도록 무적 시간을 줍니다.
		GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &AMotionControllerCharacter::DamageTimer, 0.01f, false, 1.5f);		// 타이머로 무적 시간을 비활성화합니다.
	}

	return Damage;
}

// 무적 시간 비활성화
void AMotionControllerCharacter::DamageTimer()
{
	InvincibleTimeOn = false;
}

// 위젯을 안보이게 합니다.
void AMotionControllerCharacter::DisableBloody()
{
	if (Widget->bVisible)
		Widget->bVisible = false;
}

void AMotionControllerCharacter::OnHeadOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Potion") && GrabState == E_HandState::Grab)
	{
		CurrentHp += 30;
	}
}

// 스태미너를 사용합니다.
void AMotionControllerCharacter::UseStamina(float _stamina)
{
	CurrentStamina -= _stamina;
	bIsUseStamina = true;

	GetWorld()->GetTimerManager().ClearTimer(AutoTimerHandle);			// 잠시 스태미너 회복을 멈춥니다.
	GetWorld()->GetTimerManager().SetTimer(AutoTimerHandle, this, &AMotionControllerCharacter::AutoStamina, 3.0f, false);		// 일정 시간 후, 스테미너를 다시 회복시킵니다.
}

// 스테미너를 회복합니다.
void AMotionControllerCharacter::AutoStamina()
{
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina += RecoveryPoint;

		if (CurrentStamina > MaxStamina)
		{
			CurrentStamina = MaxStamina;
		}
		GetWorld()->GetTimerManager().SetTimer(AutoTimerHandle, this, &AMotionControllerCharacter::AutoStamina, 0.01f, false);
	}
}