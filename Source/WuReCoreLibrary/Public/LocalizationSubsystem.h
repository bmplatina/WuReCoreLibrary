// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WuReGameUserSettings.h"
#include "WuReLocalizationStruct.h"
#include "WuReTypes.h"
#include "LocalizationSubsystem.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, DisplayName="WU:RE Localization Subsystem")
class WURECORELIBRARY_API ULocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// GameInstance가 Init될 때 자동 호출
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// GameInstance가 Shutdown될 때 자동 호출
	virtual void Deinitialize() override;

	// 플랫폼 등의 조건으로 아예 생성 자체를 막고 싶을 때 (선택)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	UPROPERTY(BlueprintAssignable, Category="Localization|Delegates")
	FOnBeginPlayDone OnInitializationDone;

public:
	// Language Settings
	UFUNCTION(BlueprintCallable, Category="Localization")
	void SetLanguage(const ESupportedLanguage NewLanguage);

	UFUNCTION(BlueprintPure, Category="Localization")
	ESupportedLanguage GetCurrentLanguage() const;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocaleChanged, ESupportedLanguage, NewLanguage);
	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLocaleChanged OnLocaleChanged;
	
	UFUNCTION(BlueprintPure, Category="Localization")
	FText GetLocalizedText(const FString& InCode, const ELocalizationType Type) const;
	
	UFUNCTION(BlueprintPure, Category="Localization")
	FWuReLocalizationStruct GetLocalizedContextStruct(const FName RowName, const ELocalizationType Type) const;
	
protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Localization")
	void K2Node_OnInitializationDone();

private:
	UPROPERTY()
	TObjectPtr<class UWuReGameUserSettings> UserSettings;
	
	UPROPERTY(EditAnywhere, Category="Localization", meta=(AllowPrivateAccess=true), DisplayName="Localization Table for UI")
	TObjectPtr<UDataTable> GlobalUILocalizationTable;
	UPROPERTY(EditAnywhere, Category="Localization", meta=(AllowPrivateAccess=true), DisplayName="Localization Table for Dialogue")
	TObjectPtr<UDataTable> GlobalDialogueLocalizationTable;
	
	ESupportedLanguage CurrentLanguage = ESupportedLanguage::EN_GB;
};
