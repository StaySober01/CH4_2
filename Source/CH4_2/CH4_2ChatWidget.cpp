#include "CH4_2ChatWidget.h"
#include "CH4_2PlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

void UCH4_2ChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (MessageInput)
	{
		MessageInput->SetClearKeyboardFocusOnCommit(false);
		MessageInput->OnTextCommitted.AddUniqueDynamic(this, &UCH4_2ChatWidget::HandleTextCommitted);
	}
	EndChat();
}

void UCH4_2ChatWidget::NativeDestruct()
{
	bEditing = false;
	if (MessageInput)
	{
		MessageInput->OnTextCommitted.RemoveDynamic(this, &UCH4_2ChatWidget::HandleTextCommitted);
	}
	Super::NativeDestruct();
}

bool UCH4_2ChatWidget::BeginChat()
{
	if (!MessageInput)
	{
		return false;
	}
	MessageInput->SetText(FText::GetEmpty());
	MessageInput->SetVisibility(ESlateVisibility::Visible);
	bEditing = true;
	return true;
}

void UCH4_2ChatWidget::EndChat()
{
	bEditing = false; // Commit events caused by focus changes must not submit twice.
	if (MessageInput)
	{
		MessageInput->SetText(FText::GetEmpty());
		MessageInput->SetVisibility(ESlateVisibility::Hidden);
	}
}

TSharedPtr<SWidget> UCH4_2ChatWidget::GetInputFocusTarget()
{
	return MessageInput ? MessageInput->TakeWidget() : TakeWidget();
}

void UCH4_2ChatWidget::SetHistory(const TArray<FText>& Messages)
{
	if (!ChatHistory)
	{
		return;
	}
	FString Joined;
	for (const FText& Message : Messages)
	{
		if (!Joined.IsEmpty())
		{
			Joined += TEXT("\n");
		}
		Joined += Message.ToString();
	}
	ChatHistory->SetText(FText::FromString(Joined));
	if (ChatScroll)
	{
		ChatScroll->ScrollToEnd();
	}
}

void UCH4_2ChatWidget::SetFeedback(const FText& Message)
{
	if (FeedbackText)
	{
		FeedbackText->SetText(Message);
	}
}

void UCH4_2ChatWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!bEditing)
	{
		return;
	}
	if (ACH4_2PlayerController* Controller = Cast<ACH4_2PlayerController>(GetOwningPlayer()))
	{
		if (CommitMethod == ETextCommit::OnEnter)
		{
			Controller->SubmitChatMessage(Text.ToString());
		}
		Controller->CloseChat();
	}
}

FReply UCH4_2ChatWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bEditing && InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (ACH4_2PlayerController* Controller = Cast<ACH4_2PlayerController>(GetOwningPlayer()))
		{
			Controller->CloseChat();
		}
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}
