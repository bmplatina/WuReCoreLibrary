// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "WuReTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WuReStatics.generated.h"

class UWuReUserWidget;
class UWuReLevelLoadingWidget;
/**
 * 
 */
UCLASS(DisplayName="WU:RE Gameplay Statics")
class WURECORELIBRARY_API UWuReStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Level Streaming", meta=(WorldContext="WorldContextObject"))
	static bool LoadLevelByRowName(const UObject* WorldContextObject,
	                               TSubclassOf<UWuReLevelLoadingWidget> LoadingWidgetClass,
	                               const FName LevelRowName, const FVector WorldLocationOffset = FVector::ZeroVector,
	                               const FRotator WorldRotationOffset = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Create WU:RE Widget and Add to Viewport")
	static UWuReUserWidget* CreateAndAddWuReWidgetToViewport(const UObject* WorldContextObject,
	                                                         TSubclassOf<UWuReUserWidget> InWidgetClass,
	                                                         EInputMode InputMode = EInputMode::GameAndWidget);

	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Remove WU:RE Widget from Parent")
	static void RemoveWuReWidgetFromParent(const UObject* WorldContextObject,
	                                                   UWuReUserWidget* InWidget,
	                                                   EInputMode PostInputMode = EInputMode::GameAndWidget);
};
