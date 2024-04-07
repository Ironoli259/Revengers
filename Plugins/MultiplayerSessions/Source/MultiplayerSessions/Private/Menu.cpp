// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

/**
 * This function is called when the Menu is setup
 * @param NumberOfPublicConnections The number of public connections the session will have
 * @param TypeOfMatch The type of match the session will be
 */
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	this->PathToLobby = FString::Printf(TEXT("%s?listen?"), *LobbyPath);
	this->NumPublicConnections = NumberOfPublicConnections;
	this->MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if(UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

/**
 * This function is called when the Menu is initialized
 * @return Whether the initialization was successful or not
 */
bool UMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	
	if(HostButton)
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	
	if(JoinButton)
		JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
	
	
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	
	Super::NativeDestruct();
}

/**
 * This function is called when the CreateSession function is called in the MultiplayerSessionsSubsystem
 * @param bWasSuccessful Whether the session was created successfully or not
 */
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("Failed to Create Session!"));
		HostButton->SetIsEnabled(true);
		return;
	}
	
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("Session Created Successfully!"));

	if(UWorld* World = GetWorld())
	{
		World->ServerTravel(PathToLobby);
	}
}

/**
 * This function is called when the FindSessions function is called in the MultiplayerSessionsSubsystem
 * @param SearchResults The results of the search
 * @param bWasSuccessful Whether the search was successful or not
 */
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
		return;
	
	for (auto Result : SearchResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if(SettingsValue == MatchType)
		{
			if(GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("Found a session with the correct match type!"));
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if(!bWasSuccessful || SearchResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if(Subsystem)
		return;
	
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if(SessionInterface.IsValid())
	{
		FString Address;
		SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if(PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}

	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if(!GEngine) return;
    
	if(bWasSuccessful)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Session Started Successfully"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Session Start Failed"));
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			const FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
