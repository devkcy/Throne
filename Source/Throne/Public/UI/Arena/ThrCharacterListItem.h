#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "ThrCharacterListItem.generated.h"

UCLASS()
class THRONE_API UThrCharacterListItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* InListItemData) override;
	virtual void NativeOnItemSelectionChanged(bool bInSelected) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(meta = (BindWidget))
	class UImage* CharacterProfileImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* FocusBorder;
};

UCLASS()
class THRONE_API UThrCharacterListItemData : public UObject
{
	GENERATED_BODY()

public:
	FName CharacterName;
};
