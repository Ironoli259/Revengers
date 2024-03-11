// Fill out your copyright notice in the Description page of Project Settings.


#include "RevengersAnimInstance.h"
#include "../RevengersCharacter.h"
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
	if(RevengersCharacter == nullptr)
	{
		RevengersCharacter = Cast<ARevengersCharacter>(TryGetPawnOwner());
		if(RevengersCharacter)
		{
			RevengersChararcterMovement = RevengersCharacter->GetCharacterMovement();
		}
		else
		{
			return;
		}
	}
	
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(RevengersChararcterMovement)
	{
		// Get the lateral speed of the character from velocity
		this->GroundSpeed = UKismetMathLibrary::VSizeXY(RevengersCharacter->GetVelocity());

		// Is the character in the air?
		this->bIsFalling = RevengersChararcterMovement->IsFalling();

		// Is the character accelerating?
		if(RevengersChararcterMovement->GetCurrentAcceleration().Size() > 0.0f)
		{
			this->bIsAccelerating = true;
		}
		else
		{
			this->bIsAccelerating = false;
		}

		// Is the character aiming?
		this->bIsAiming = RevengersCharacter->GetIsAiming();

		// Does the character have a medium weapon equipped?
		this->bHasMediumWeaponEquipped = RevengersCharacter->GetHasMediumWeaponEquipped();
	}
}
