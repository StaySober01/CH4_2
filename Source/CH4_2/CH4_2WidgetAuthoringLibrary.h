#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CH4_2WidgetAuthoringLibrary.generated.h"

/** Editor authoring bridge for properties not exposed through Unreal Python. */
UCLASS()
class CH4_2_API UCH4_2WidgetAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Fills only empty widget layouts, preserves existing layouts, then compiles. */
	UFUNCTION(BlueprintCallable, Category="Editor|CH4_2")
	static bool PrepareWidgetBlueprint(UObject* Asset, bool bChat);

	/** Compile and inspect status through native editor APIs. */
	UFUNCTION(BlueprintCallable, Category="Editor|CH4_2")
	static bool CompileInterfaceBlueprint(UObject* Asset);
};
