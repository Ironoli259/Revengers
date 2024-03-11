// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevengersCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interfaces/ITargetDevice.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARevengersCharacter

ARevengersCharacter::ARevengersCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 550.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARevengersCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARevengersCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARevengersCharacter::StopSprinting);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ARevengersCharacter::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ARevengersCharacter::StopAiming);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::StartShooting);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ARevengersCharacter::StopShooting);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::StartReloading);

		// Swap Weapon
		EnhancedInputComponent->BindAction(SwapWeaponAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::StartSwappingWeapon);

		// Ability
		EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Triggered, this, &ARevengersCharacter::ActivateAbility);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

/** Move Function */
void ARevengersCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

/** Look Function */
void ARevengersCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(-(LookAxisVector.X));
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/** Function to Start Sprinting */
void ARevengersCharacter::StartSprinting()
{
	if(bIsSprinting || bIsAiming)
		return;
	this->bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	UE_LOG(LogTemp, Warning, TEXT("Start Sprinting"));
}

/** Function to Stop Sprinting */
void ARevengersCharacter::StopSprinting()
{
	this->bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	UE_LOG(LogTemp, Warning, TEXT("Stop Sprinting"));
}

/** Function to Start Aiming */
void ARevengersCharacter::StartAiming()
{
	if(this->bIsAiming)
		return;
	
	this->bIsAiming = true;
	UE_LOG(LogTemp, Warning, TEXT("Start Aiming"));

	if(bIsSprinting)
		StopSprinting();
	
	//TODO: Implement aiming logic
}

/** Function to Stop Aiming */
void ARevengersCharacter::StopAiming()
{
	if(!this->bIsAiming)
		return;
	
	this->bIsAiming = false;
	UE_LOG(LogTemp, Warning, TEXT("Stop Aiming"));
	//TODO: Implement aiming logic
}

/** Function to Start Shooting */
void ARevengersCharacter::StartShooting()
{
	if(this->bIsShooting)
		return;
	
	this->bIsShooting = true;
	UE_LOG(LogTemp, Warning, TEXT("Start Shooting"));
	//TODO: Implement shooting logic
	//TODO: Implement reloading logic
}

/** Function to Stop Shooting */
void ARevengersCharacter::StopShooting()
{
	if(!this->bIsShooting)
		return;
	
	this->bIsShooting = false;
	UE_LOG(LogTemp, Warning, TEXT("Stop Shooting"));
	//TODO: Implement shooting logic
}

void ARevengersCharacter::Shoot()
{
	//TODO: Implement shooting logic
}

/** Functions to Reload */
void ARevengersCharacter::StartReloading()
{
	if(this->bIsReloading)
		return;
	
	this->bIsReloading = true;
	UE_LOG(LogTemp, Warning, TEXT("Start Reloading"));
	//TODO: Implement reloading logic
}

void ARevengersCharacter::CompletedReload()
{
	if(!this->bIsReloading)
		return;
	
	this->bIsReloading = false;
	//TODO: Implement reloading logic
}

/** Functions to Swap Weapon */
void ARevengersCharacter::StartSwappingWeapon()
{
	if(this->bIsSwappingWeapon)
		return;
	
	this->bIsSwappingWeapon = true;
	UE_LOG(LogTemp, Warning, TEXT("Start Swapping Weapon"));
	//TODO: Implement swapping weapon logic
}

void ARevengersCharacter::CompletedSwapWeapon()
{
	if(!this->bIsSwappingWeapon)
		return;
	
	this->bIsSwappingWeapon = false;
	//TODO: Implement swapping weapon logic
}

/** Functions for Ability */
void ARevengersCharacter::ActivateAbility()
{
	//TODO: Implement ability logic
}