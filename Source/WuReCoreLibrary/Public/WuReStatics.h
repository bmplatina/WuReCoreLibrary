// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
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

	UFUNCTION(BlueprintPure, Category="Level Streaming", meta=(WorldContext="WorldContextObject"))
	static FName GetCurrentLevelRowName(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Create WU:RE Widget and Add to Viewport")
	static UWuReUserWidget* CreateAndAddWuReWidgetToViewport(const UObject* WorldContextObject,
	                                                         TSubclassOf<UWuReUserWidget> InWidgetClass,
	                                                         EInputMode InputMode = EInputMode::GameAndWidget);

	UFUNCTION(BlueprintPure, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Is Root Widget Has WU:RE Widget")
	static bool IsRootWidgetHasWuReWidget(const UObject* WorldContextObject, UWuReUserWidget* InWidget);

	UFUNCTION(BlueprintPure, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Is WU:RE Widget Activated in Viewport")
	static bool IsWuReWidgetActivatedInViewport(const UObject* WorldContextObject, UWuReUserWidget* InWidget);

	UFUNCTION(BlueprintPure, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Get Activated WU:RE Widget")
	static UWuReUserWidget* GetActivatedWuReWidget(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Get WU:RE Widget of Class")
	static UWuReUserWidget* GetWuReWidgetOfClass(const UObject* WorldContextObject,
	                                             const TSubclassOf<UWuReUserWidget> InWidgetClass);

	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Remove WU:RE Widget from Parent")
	static void RemoveWuReWidgetFromParent(const UObject* WorldContextObject,
	                                       UWuReUserWidget* InWidget);
	                                       // EInputMode PostInputMode = EInputMode::Default);

	/**
	 * InWidgetClass 위젯이 게임플레이 스택에 열려 있으면 닫고, 없으면 생성해 엽니다.
	 * 닫은 뒤 스택이 비면 게임 전용 입력(커서 숨김, 마우스 캡처)으로 되돌립니다.
	 * @param bIsWidgetOpened 이번 호출로 위젯이 열렸으면 true, 닫혔으면 false
	 * @param bLockMouse 열 때 입력 모드를 GameOnly로 할지 여부 (false면 GameAndWidget)
	 * @return 열린 위젯. 닫혔거나 실패하면 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Create WU:RE Widget by FlipFlop")
	static UWuReUserWidget* ToggleWuReWidget(const UObject* WorldContextObject,
	                             TSubclassOf<UWuReUserWidget> InWidgetClass,
	                             bool& bIsWidgetOpened, EInputMode InputMode = EInputMode::GameAndWidget);

	UFUNCTION(BlueprintCallable, Category="Widget", meta=(WorldContext="WorldContextObject"),
		DisplayName="Remove All WU:RE Widgets from Parent")
	static void RemoveAllWuReWidgets(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Localization", meta=(WorldContext="WorldContextObject"),
		DisplayName="Get Localized UI Text From Row Name")
	static FText GetLocalizedUIText(const UObject* WorldContextObject, FName RowName, const ELocalizationType Type);

	UFUNCTION(BlueprintPure, Category="Localization", meta=(WorldContext="WorldContextObject"))
	static FText FormatTextOneParam(const UObject* WorldContextObject, FText Text, FString Param, FText ToReplaceParam);
	
	UFUNCTION(BlueprintCallable, Category="Localization|Subtitle", meta=(WorldContext="WorldContextObject"))
	static bool SendSubtitle(const UObject* WorldContextObject, FName InRowNameForDialog, AActor* ActorToAttachAudio);
	
	UFUNCTION(BlueprintPure, Category="Graphics")
	static bool IsHardwareRayTracingSupported();

	// 지금 실제로 RT가 활성 상태인가 (프로젝트/옵션/CVar 포함)
	UFUNCTION(BlueprintPure, Category="Graphics")
	static bool IsRayTracingActive();
	
	UFUNCTION(BlueprintCallable, Category="Graphics")
	static void SetRayTracingActive(const bool bActive);
};
