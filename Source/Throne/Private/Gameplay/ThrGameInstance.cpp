#include "Gameplay/ThrGameInstance.h"
#include "Gamedata/ThrDataManager.h"
#include "Gameplay/ThrGameMode.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Online.h"

FOnFindSessions UThrGameInstance::OnFindSessions;
FOnNotifyLobbyMessage UThrGameInstance::OnNotifyLobbyMessage;

void UThrGameInstance::Init()
{
	Super::Init();

	DataManager = NewObject<UThrDataManager>();

	SessionInterface = Online::GetSessionInterfaceChecked();
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UThrGameInstance::CompleteFindSessions);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UThrGameInstance::CompleteJoinSession);
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 10;

		if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
			SessionSearch->bIsLanQuery = OnlineSubsystem->GetSubsystemName() == TEXT("NULL");

		FOnlineSearchSettings SearchSettings;
		SearchSettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings = SearchSettings;
	}
}

const TArray<FOnlineSessionSearchResult> UThrGameInstance::GetSessionSearchResults() const
{
	if (SessionSearch.IsValid())
		return SessionSearch->SearchResults;

	return TArray<FOnlineSessionSearchResult>();
}

void UThrGameInstance::TryToCreateSession(const TMap<FName, FString>& InSessionSettingParams)
{
	if (SessionInterface.IsValid())
		SessionInterface->DestroySession(NAME_GameSession);

	SessionName = FName(InSessionSettingParams.FindRef(TEXT("SessionName")));
	bLeftSide = InSessionSettingParams.FindRef(TEXT("Side")) == TEXT("Left");
	TeamName = FName(InSessionSettingParams.FindRef(TEXT("TeamName")));

	CreateSession(InSessionSettingParams);
}

void UThrGameInstance::TryToFindSessions()
{
	if (SessionInterface.IsValid() && SessionSearch.IsValid() && CheckValidUserID())
	{
		SessionInterface->FindSessions(*UserID, SessionSearch.ToSharedRef());
		return;
	}

	OnFindSessions.ExecuteIfBound(false);
}

void UThrGameInstance::TryToJoinSession(const FOnlineSessionSearchResult& InSessionSearchResult, const FName InTeamName)
{
	const auto& SessionSettings = InSessionSearchResult.Session.SessionSettings.Settings;

	auto* JoinSessionName = SessionSettings.Find(TEXT("SessionName"));
	if (SessionInterface.IsValid() && JoinSessionName && CheckValidUserID())
	{
		SessionInterface->DestroySession(NAME_GameSession);

		SessionName = FName(JoinSessionName->Data.ToString());
		bLeftSide = SessionSettings.FindRef(TEXT("Side")) != TEXT("Left");
		TeamName = InTeamName;

		SessionInterface->JoinSession(*UserID, NAME_GameSession, InSessionSearchResult);
		return;
	}
	
	OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Failed to join the match"));
}

void UThrGameInstance::CompleteCreateSession(const bool bInSucceeded) const
{
	auto* World = GetWorld();
	if (bInSucceeded && World)
	{
		World->ServerTravel(TEXT("/Game/Maps/Arena/Arena?listen"));
		return;
	}

	OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Failed to create the match"));
}

void UThrGameInstance::CompleteFindSessions(const bool bInSucceeded) const
{
	OnFindSessions.ExecuteIfBound(bInSucceeded);
}

void UThrGameInstance::CompleteJoinSession(const FName InSessionName, const EOnJoinSessionCompleteResult::Type InResult) const
{
	if (InResult == EOnJoinSessionCompleteResult::Success)
	{
		auto* Player = GetFirstLocalPlayerController();
		if (SessionInterface.IsValid() && Player)
		{
			FString TravelURL;
			if (SessionInterface->GetResolvedConnectString(InSessionName, TravelURL))
			{
				Player->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
				return;
			}
		}
	}

	switch (InResult)
	{
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Already joined the match"));
		return;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Can't retrieve the match"));
		return;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		OnNotifyLobbyMessage.ExecuteIfBound(TEXT("The match doesn't exist"));
		return;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		OnNotifyLobbyMessage.ExecuteIfBound(TEXT("The match is full"));
		return;
	case EOnJoinSessionCompleteResult::UnknownError:
	default:
		OnNotifyLobbyMessage.ExecuteIfBound(TEXT("Failed to join the match"));
	}
}

const bool UThrGameInstance::CheckValidUserID()
{
	if (!UserID.IsValid())
	{
		if (auto* Player = GetFirstLocalPlayerController(GetWorld()))
		{
			if (APlayerState* PlayerState = Player->PlayerState)
				UserID = PlayerState->GetUniqueId().GetUniqueNetId();
		}
	}

	return UserID.IsValid();
}
