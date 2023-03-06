#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "ThrCheckListItem.generated.h"

UCLASS()
class THRONE_API UThrCheckListItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* InListItemData) override;
	virtual void NativeOnItemSelectionChanged(bool bInSelected) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CheckBoxText;

	UPROPERTY(meta = (BindWidget))
	class UImage* CheckImage;
};

UCLASS()
class THRONE_API UThrCheckListItemData : public UObject
{
	GENERATED_BODY()

public:
	FString CheckBoxText;
	FLinearColor TextColor;
};
