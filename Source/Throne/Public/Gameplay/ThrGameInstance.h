#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ThrGameInstance.generated.h"

namespace EOnJoinSessionCompleteResult { enum Type; }

DECLARE_DELEGATE_OneParam(FOnFindSessions, const bool);
DECLARE_DELEGATE_OneParam(FOnNotifyLobbyMessage, const FString&);

UCLASS()
class THRONE_API UThrGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	static FOnFindSessions OnFindSessions;
	static FOnNotifyLobbyMessage OnNotifyLobbyMessage;

	virtual void Init() override;

	const bool IsLeftSide() const { return bLeftSide; }
	const FName GetTeamName() const { return TeamName; }

	const TArray<class FOnlineSessionSearchResult> GetSessionSearchResults() const;

	void TryToCreateSession(const TMap<FName, FString>& InSessionSettingParams);
	void TryToFindSessions();
	void TryToJoinSession(const class FOnlineSessionSearchResult& InSessionSearchResult, const FName InTeamName);
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void CreateSession(const TMap<FName, FString>& InSessionSettingParams) const;
	
	UFUNCTION(BlueprintCallable)
	void CompleteCreateSession(const bool bInSucceeded) const;

private:
	void CompleteFindSessions(const bool bInSucceeded) const;
	void CompleteJoinSession(const FName InSessionName, const EOnJoinSessionCompleteResult::Type InResult) const;

	const bool CheckValidUserID();

	UPROPERTY(Transient)
	class UThrDataManager* DataManager;

	TSharedPtr<class IOnlineSession> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	TSharedPtr<const class FUniqueNetId> UserID;

	FName SessionName;
	bool bLeftSide;
	FName TeamName;
};
