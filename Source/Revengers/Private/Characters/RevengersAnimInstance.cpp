// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RevengersAnimInstance.h"
#include "Characters/RevengersCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Set up the animation instance - START
void URevengersAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RevengersCharacter = Cast<ARevengersCharacter>(TryGetPawnOwner());
	if(RevengersCharacter)
	{
		RevengersChararcterMovement = RevengersCharacter->GetCharacterMovement();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No character found in the animation instance!"));
	}
}

// Update the animation instance - UPDATE
void URevengersAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if(RevengersCharacter == nullptr)
		RevengersCharacter = Cast<ARevengersCharacter>(TryGetPawnOwner());

	if(RevengersCharacter == nullptr) return;

	if(RevengersChararcterMovement)
	{
		// Get the lateral speed of the character from velocity
		this->Speed = UKismetMathLibrary::VSizeXY(RevengersCharacter->GetVelocity());

		// Is the character in the air?
		this->bIsInAir = RevengersChararcterMovement->IsFalling();

		// Is the character accelerating?
		this->bIsAccelerating = RevengersChararcterMovement->GetCurrentAcceleration().Size() > 0.0f ? true : false;

		// Is the character aiming?
		this->bIsAiming = RevengersCharacter->GetIsAiming();

		// Is the character shooting?
		this->bIsShooting = RevengersCharacter->GetIsShooting();

		// Does the character have a medium weapon equipped?
		this->bHasMediumWeaponEquipped = RevengersCharacter->GetHasMediumWeaponEquipped();
	}
}
