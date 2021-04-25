#include "Dog.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"

#include "Kismet/GameplayStatics.h"
#include "MyCharacter/MotionControllerCharacter.h"

#include "Monster/Dog/DogAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprint.h"

#include "Camera/CameraComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "HandMotionController/RightHandMotionController.h"
#include "HandMotionController/LeftHandMotionController.h"
#include "Components/SphereComponent.h"

#include "Equipment/PlayerSword.h"

// 플레이어를 감지하면 쫓아와서 팔을 무는 몬스터입니다.

ADog::ADog()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메쉬를 찾아서 적용합니다.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MonsterMesh(TEXT("SkeletalMesh'/Game/Assets/CharacterEquipment/Monster/Dog/Mesh2/MON_DOG_MESH.MON_DOG_MESH'"));
	if (MonsterMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MonsterMesh.Object);
	}

	// 행동트리를 찾아서 적용합니다.
	static ConstructorHelpers::FObjectFinder<UBehaviorTree>Monster_BehaviorTree(TEXT("BehaviorTree'/Game/Blueprints/Monster/Dog/AI/RagdollDogBT_2.RagdollDogBT_2'"));
	if (Monster_BehaviorTree.Succeeded())
	{
		BehaviorTree = Monster_BehaviorTree.Object;
	}

	// 애니메이션을 찾아서 적용합니다.
	static ConstructorHelpers::FObjectFinder<UClass>Monster_AnimBlueprint(TEXT("AnimBlueprint'/Game/Blueprints/Monster/Dog/Blueprints2/ABP_Dog_3.ABP_Dog_3_C'"));
	if (Monster_AnimBlueprint.Succeeded())
	{
		UClass* DogAnimBlueprint = Monster_AnimBlueprint.Object;

		if (DogAnimBlueprint)
		{
			GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			GetMesh()->SetAnimInstanceClass(DogAnimBlueprint);
		}
	}

	// 머티리얼을 찾아서 적용합니다.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>Monster_Material(TEXT("Material'/Game/Assets/CharacterEquipment/Monster/Dog/Materials/M_Dog.M_Dog'"));
	if (Monster_Material.Succeeded())
	{
		GetMesh()->SetMaterial(0, Monster_Material.Object);
	}

	// 몬스터의 머리콜리전을 생성합니다.
	DogAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DogAttack"));
	DogAttackCollision->SetupAttachment(GetMesh(), TEXT("HeadSocket"));

	// 감각 컴포넌트를 적용합니다.
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));

	// AI 컨트롤러를 적용합니다.
	AIControllerClass = ADogAIController::StaticClass();

	// 캡슐의 크기와 메쉬의 크기와 방향을 설정합니다.
	GetCapsuleComponent()->SetCapsuleHalfHeight(55.0f);
	GetCapsuleComponent()->SetCapsuleRadius(30.0f);
	GetCapsuleComponent()->SetCollisionProfileName("IgnoreOnlyPawn");
	GetMesh()->SetRelativeLocation(FVector(-20.0f, 0.0f, -55.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName("Ragdoll");

	// 머리 콜리전의 크기와 위치를 설정합니다.
	DogAttackCollision->SetCollisionProfileName(TEXT("OverlapAll"));
	DogAttackCollision->SetRelativeLocation(FVector(10.0f, -10.0f, 0.0f));
	DogAttackCollision->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	DogAttackCollision->SetActive(false);
	DogAttackCollision->ComponentTags.Add("DogAttackCollision");
	DogAttackCollision->bGenerateOverlapEvents = false;

	// 감각의 요소를 설정합니다.
	PawnSensing->bHearNoises = false;
	PawnSensing->bSeePawns = true;
	PawnSensing->SetPeripheralVisionAngle(40.0f);
	PawnSensing->SightRadius = 1200.0f;
	PawnSensing->SensingInterval = 0.1f;

	bOnLand = true;
	Landing = false;

	MaxHP = 1.0f;
	CurrentHP = MaxHP;
	bIsDeath = false;
	bIsDetach = false;

	AttackWaite = false;

	Tags.Add("Monster");
	Tags.Add("Dog");
	Tags.Add(FName(TEXT("DisregardForLeftHand")));
	Tags.Add(FName(TEXT("DisregardForRightHand")));
}

void ADog::BeginPlay()
{
	Super::BeginPlay();

	// 상태 초기화
	CurrentDogState = EDogState::Idle;
	CurrentDogAnimState = EDogAnimState::Idle;
	CurrentDogBattleState = EDogBattleState::Nothing;
	CurrentDogAirState = EDogAirState::Nothing;
	CurrentDogJumpState = EDogJumpState::Nothing;
	CurrentDogCircleState = EDogCircleState::Nothing;

	// 머리와 오버랩시 실행할 함수를 등록합니다.
	DogAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &ADog::OnAttackCollisionOverlap);

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &ADog::OnSeePlayer);
	}
}

void ADog::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AI = Cast<ADogAIController>(GetController());
	FFindFloorResult FloorDistance;
	GetCharacterMovement()->ComputeFloorDist(GetCapsuleComponent()->GetComponentLocation(), 10000.0f, 10000.0f, FloorDistance, 34.0f);

	// 블랙보드에 정보를 전달합니다.
	if (AI)
	{
		AI->BBComponent->SetValueAsEnum("CurrentDogState", (uint8)CurrentDogState);
		AI->BBComponent->SetValueAsEnum("CurrentDogAnimState", (uint8)CurrentDogAnimState);
		AI->BBComponent->SetValueAsEnum("CurrentDogJumpState", (uint8)CurrentDogJumpState);
		AI->BBComponent->SetValueAsEnum("CurrentDogCircleState", (uint8)CurrentDogCircleState);
		AI->BBComponent->SetValueAsEnum("CurrentDogBattleState", (uint8)CurrentDogBattleState);
		AI->BBComponent->SetValueAsEnum("CurrentDogAirState", (uint8)CurrentDogAirState);
		AI->BBComponent->SetValueAsObject("AttachActor", AttachActor);
		AI->BBComponent->SetValueAsBool("bOnLand", bOnLand);
		AI->BBComponent->SetValueAsBool("DeathFlag", bIsDeath);
		AI->BBComponent->SetValueAsFloat("HP", CurrentHP);
		height = FloorDistance.FloorDist;
		CurrentFalling = GetCharacterMovement()->IsFalling();
	}

}

void ADog::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// 플레이어 감지
void ADog::OnSeePlayer(APawn* Pawn)
{
	FFindFloorResult FloorDistance;;
	// 땅과의 거리를 체크합니다.
	GetCharacterMovement()->ComputeFloorDist(GetCapsuleComponent()->GetComponentLocation(), 10000.0f, 10000.0f, FloorDistance, 34, 0);

	// 감지한 액터의 태그가 'Character'일 때, 그 액터는 몬스터의 타깃이 됩니다.
	if (Pawn->ActorHasTag("Character") && FloorDistance.FloorDist < 3.0f)
	{
		ADogAIController* AI = Cast<ADogAIController>(GetController());

		if (AI && !AI->BBComponent->GetValueAsObject("Player"))
		{
			// 상태 설정
			Target = Pawn;
			CurrentDogState = EDogState::Chase;
			CurrentDogAnimState = EDogAnimState::Run;
			CurrentDogJumpState = EDogJumpState::Nothing;
			CurrentDogCircleState = EDogCircleState::Nothing;
			AI->BBComponent->SetValueAsObject("Player", Pawn);
			GetCharacterMovement()->MaxWalkSpeed = 550.0f;
		}
	}
}

// 몬스터의 머리와 플레이어의 머리가 오버랩되면 팔을 물게 됩니다.
void ADog::OnAttackCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->ComponentHasTag("Head"))
	{
		UGameplayStatics::ApplyDamage(OtherActor, 20, UGameplayStatics::GetPlayerController(GetWorld(), 0), this, nullptr);		// 오버랩된 액터에 데미지를 줍니다.

		AMotionControllerCharacter* Character = Cast<AMotionControllerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		if (Character)
		{
			ARightHandMotionController* RightController = Cast<ARightHandMotionController>(Character->RightHand);

			// 플레이어 팔에 몬스터가 없으면 팔에 붙입니다.
			if (!RightController->AttachDog)
			{
				RightController->AttachDog = this;

				DogAttackCollision->bGenerateOverlapEvents = false;			// 머리 콜리전 비활성화

				// 붙입니다.
				FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
				AttachToComponent(RightController->AttachDogPosition, AttachRules);

				// 개의 특정 부위이하로 전부 레그돌화 시킵니다.
				GetMesh()->SetAllBodiesBelowSimulatePhysics("Bip002-Spine1", true, true);
				GetCapsuleComponent()->SetCapsuleRadius(10.0f);
				GetCapsuleComponent()->SetCapsuleHalfHeight(10.0f);

				SetActorRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
				SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

				AttachActor = RightController;
				AI->BBComponent->SetValueAsBool("bIsBiting", true);
			}
		}
	}
}

// 데미지를 받습니다.
float ADog::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	APlayerSword* Sword = Cast<APlayerSword>(DamageCauser);
	if (CurrentDogState == EDogState::Bite && Sword)
	{
		return 0;
	}

	CurrentHP -= Damage;

	if (CurrentHP < 0.0f)
	{
		DogAttackCollision->bGenerateOverlapEvents = false;

		if (CurrentDogState == EDogState::Bite)
		{
			CurrentDogAnimState = EDogAnimState::FallingDeath;
			bIsDeath = true;
			bpunchDetach = true;
		}
		else if (CurrentDogState == EDogState::Battle)
		{
			CurrentDogState = EDogState::Death;
		}
	}

	return Damage;
}