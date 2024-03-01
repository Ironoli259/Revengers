// Fill out your copyright notice in the Description page of Project Settings.


#include "RevengersAnimInstance.h"
#include "../RevengersCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void URevengersAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RevengersCharacter = Cast<ARevengersCharacter>(TryGetPawnOwner());
	if(RevengersCharacter)
	{
		RevengersChararcterMovement = RevengersCharacter->GetCharacterMovement();
	}
}

void URevengersAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(RevengersChararcterMovement)
	{
		this->GroundSpeed = UKismetMathLibrary::VSizeXY(RevengersCharacter->GetVelocity());
		this->IsFalling = RevengersChararcterMovement->IsFalling();
	}
}
