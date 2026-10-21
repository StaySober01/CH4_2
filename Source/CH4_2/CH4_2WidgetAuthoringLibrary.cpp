#include "CH4_2WidgetAuthoringLibrary.h"
#include "CH4_2.h"

#if WITH_EDITOR
#include "CH4_2ChatWidget.h"
#include "CH4_2PlayerStatsWidget.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

namespace
{
	UTextBlock* MakeText(UWidgetTree* Tree, FName Name, const TCHAR* Value, bool bVariable = false)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Text->bIsVariable = bVariable;
		Text->SetText(FText::FromString(Value));
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 18;
		Text->SetFont(Font);
		return Text;
	}

	bool RequireWidget(UWidgetTree* Tree, FName Name, UClass* Type)
	{
		UWidget* Widget = Tree->FindWidget(Name);
		if (!Widget || !Widget->IsA(Type) || !Widget->bIsVariable)
		{
			UE_LOG(LogCH4_2, Error, TEXT("WBP requires %s of type %s with Is Variable enabled. Existing layout was preserved."), *Name.ToString(), *Type->GetName());
			return false;
		}
		return true;
	}
}
#endif

bool UCH4_2WidgetAuthoringLibrary::PrepareWidgetBlueprint(UObject* Asset, bool bChat)
{
#if WITH_EDITOR
	UWidgetBlueprint* Blueprint = Cast<UWidgetBlueprint>(Asset);
	UClass* ExpectedParent = bChat ? UCH4_2ChatWidget::StaticClass() : UCH4_2PlayerStatsWidget::StaticClass();
	if (!Blueprint || Blueprint->ParentClass != ExpectedParent || !Blueprint->WidgetTree)
	{
		UE_LOG(LogCH4_2, Error, TEXT("Unexpected widget Blueprint/parent: %s"), *GetNameSafe(Asset));
		return false;
	}
	UWidgetTree* Tree = Blueprint->WidgetTree;
	UCanvasPanel* ExistingCanvas = Cast<UCanvasPanel>(Tree->RootWidget);
	const bool bEmpty = !Tree->RootWidget || (ExistingCanvas && ExistingCanvas->GetChildrenCount() == 0);
	if (bEmpty)
	{
		Blueprint->Modify();
		Tree->Modify();
		// Reuse the empty factory-created canvas, including one left by a failed Python run.
		UCanvasPanel* Root = ExistingCanvas ? ExistingCanvas : Tree->ConstructWidget<UCanvasPanel>();
		Root->Modify();
		Tree->RootWidget = Root;
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UBorder* Panel = Tree->ConstructWidget<UBorder>();
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.8f));
		Panel->SetPadding(FMargin(12.0f));
		UCanvasPanelSlot* Slot = Root->AddChildToCanvas(Panel);
		UVerticalBox* Rows = Tree->ConstructWidget<UVerticalBox>();
		Panel->SetContent(Rows);
		if (bChat)
		{
			Slot->SetAnchors(FAnchors(0.0f, 1.0f));
			Slot->SetAlignment(FVector2D(0.0f, 1.0f));
			Slot->SetPosition(FVector2D(24.0f, -24.0f));
			Slot->SetSize(FVector2D(520.0f, 290.0f));
			Rows->AddChildToVerticalBox(MakeText(Tree, TEXT("ChatTitle"), TEXT("Chat | Enter: type/send | Esc: cancel")));
			USizeBox* HistorySize = Tree->ConstructWidget<USizeBox>();
			HistorySize->SetHeightOverride(150.0f);
			Rows->AddChildToVerticalBox(HistorySize);
			UScrollBox* Scroll = Tree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ChatScroll"));
			Scroll->bIsVariable = true;
			HistorySize->SetContent(Scroll);
			UTextBlock* History = MakeText(Tree, TEXT("ChatHistory"), TEXT(""), true);
			History->SetAutoWrapText(true);
			History->SetWrapTextAt(470.0f);
			Scroll->AddChild(History);
			UEditableTextBox* Entry = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("MessageInput"));
			Entry->bIsVariable = true;
			Entry->SetHintText(FText::FromString(TEXT("Message (max 200 characters)")));
			Entry->SetVisibility(ESlateVisibility::Hidden);
			Rows->AddChildToVerticalBox(Entry);
			UTextBlock* Feedback = MakeText(Tree, TEXT("FeedbackText"), TEXT(""), true);
			Feedback->SetAutoWrapText(true);
			Rows->AddChildToVerticalBox(Feedback);
		}
		else
		{
			Slot->SetPosition(FVector2D(24.0f, 24.0f));
			Slot->SetAutoSize(true);
			Rows->AddChildToVerticalBox(MakeText(Tree, TEXT("CoinsText"), TEXT("Coins: 0"), true));
			Rows->AddChildToVerticalBox(MakeText(Tree, TEXT("BonusSpeedText"), TEXT("Bonus max speed: 0"), true));
			Rows->AddChildToVerticalBox(MakeText(Tree, TEXT("MatchText"), TEXT("Waiting for players: 1/2"), true));
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}

	// Nonempty layouts are never replaced, even if incomplete.
	const bool bBindingsValid = bChat
		? RequireWidget(Tree, TEXT("ChatHistory"), UTextBlock::StaticClass())
			&& RequireWidget(Tree, TEXT("ChatScroll"), UScrollBox::StaticClass())
			&& RequireWidget(Tree, TEXT("MessageInput"), UEditableTextBox::StaticClass())
			&& RequireWidget(Tree, TEXT("FeedbackText"), UTextBlock::StaticClass())
		: RequireWidget(Tree, TEXT("CoinsText"), UTextBlock::StaticClass())
			&& RequireWidget(Tree, TEXT("BonusSpeedText"), UTextBlock::StaticClass())
			&& RequireWidget(Tree, TEXT("MatchText"), UTextBlock::StaticClass());
	return bBindingsValid && CompileInterfaceBlueprint(Blueprint);
#else
	return false;
#endif
}

bool UCH4_2WidgetAuthoringLibrary::CompileInterfaceBlueprint(UObject* Asset)
{
#if WITH_EDITOR
	UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
	if (!Blueprint)
	{
		return false;
	}
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	if (Blueprint->Status == BS_UpToDate || Blueprint->Status == BS_UpToDateWithWarnings)
	{
		return true;
	}
	UE_LOG(LogCH4_2, Error, TEXT("Blueprint compilation failed: %s. See its Compiler Results."), *Blueprint->GetPathName());
#endif
	return false;
}
