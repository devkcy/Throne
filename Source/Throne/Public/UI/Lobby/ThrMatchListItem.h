#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Online.h"
#include "ThrMatchListItem.generated.h"

UCLASS()
class THRONE_API UThrMatchListItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* InListItemData) override;
	virtual void NativeOnItemSelectionChanged(bool bInSelected) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SideText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TeamNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CreationTimeText;

	UPROPERTY(meta = (BindWidget))
	class UImage* PlayerProfileImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* FocusBorder;
};

UCLASS()
class THRONE_API UThrMatchListItemData : public UObject
{
	GENERATED_BODY()

public:
	FOnlineSessionSearchResult SessionSearchResult;
};
