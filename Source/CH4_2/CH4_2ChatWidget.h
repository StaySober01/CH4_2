#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "CH4_2ChatWidget.generated.h"

class UEditableTextBox;
class UScrollBox;
class UTextBlock;

/** WBP_Chat owns layout; this class handles text and input events. */
UCLASS(Abstract)
class CH4_2_API UCH4_2ChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	bool BeginChat();
	void EndChat();
	void SetHistory(const TArray<FText>& Messages);
	void SetFeedback(const FText& Message);
	TSharedPtr<SWidget> GetInputFocusTarget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ChatHistory;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> ChatScroll;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> MessageInput;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> FeedbackText;

	bool bEditing = false;

	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
