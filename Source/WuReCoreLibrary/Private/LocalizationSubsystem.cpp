// © Platina Production 2017~2024


#include "LocalizationSubsystem.h"

#include "WuReGameUserSettings.h"
#include "Internationalization/Culture.h"

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	OnInitializationDone.AddDynamic(this, &ULocalizationSubsystem::K2Node_OnInitializationDone);
	
	UserSettings = Cast<UWuReGameUserSettings>(UGameUserSettings::GetGameUserSettings());
	FCulturePtr CultureLocal = FInternationalization::Get().GetCurrentCulture();
	FString Language = CultureLocal->GetTwoLetterISOLanguageName();

	if (IsValid(UserSettings))
	{
		if (UserSettings->bIsInitLoad) // 최초 실행 시 OS 언어를 적용
		{
			if (Language == TEXT("ko"))
				UserSettings->CurrentLanguage = ESupportedLanguage::KO_KR;
			else if (Language == TEXT("en"))
				UserSettings->CurrentLanguage = ESupportedLanguage::EN_GB;
			else
				UserSettings->CurrentLanguage = ESupportedLanguage::EN_GB;
			UserSettings->ApplySettings();
		}
		UserSettings->bIsInitLoad = false;
		CurrentLanguage = UserSettings->CurrentLanguage;
		WURELOG(Log, TEXT("Current Language : %s"), *Language);
	}
	
	OnLocaleChanged.Broadcast(CurrentLanguage);
	OnInitializationDone.Broadcast();
}

void ULocalizationSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool ULocalizationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!IsValid(GlobalUILocalizationTable) || !IsValid(GlobalDialogueLocalizationTable)) return false;

	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);	
	return ChildClasses.Num() == 0  ? Super::ShouldCreateSubsystem(Outer) : false;
}

void ULocalizationSubsystem::SetLanguage(const ESupportedLanguage NewLanguage)
{
	CurrentLanguage = NewLanguage;

	UserSettings->CurrentLanguage = NewLanguage;
	UserSettings->SaveConfig();

	OnLocaleChanged.Broadcast(NewLanguage);
}

ESupportedLanguage ULocalizationSubsystem::GetCurrentLanguage() const
{
	return UserSettings->CurrentLanguage;
}

FText ULocalizationSubsystem::GetLocalizedText(const FString& InCode, const ELocalizationType Type) const
{
	FText LocalizedText = FText::GetEmpty();
	if (CurrentLanguage == ESupportedLanguage::KO_KR) LocalizedText = GetLocalizedContextStruct(FName(*InCode), Type).ContextKoKr;
	else if (CurrentLanguage == ESupportedLanguage::EN_GB) LocalizedText = GetLocalizedContextStruct(FName(*InCode), Type).ContextEnGb;
	else LocalizedText = GetLocalizedContextStruct(FName(*InCode), Type).ContextEnGb;
	return LocalizedText;
}

FWuReLocalizationStruct ULocalizationSubsystem::GetLocalizedContextStruct(const FName RowName, const ELocalizationType Type) const
{
	FString ContextString(TEXT("GENERAL"));
	TObjectPtr<UDataTable> LocalizationTable = Type == ELocalizationType::Dialogue ? GlobalDialogueLocalizationTable : GlobalUILocalizationTable;
	if(IsValid(LocalizationTable))
	{
		if(FWuReLocalizationStruct* Row = LocalizationTable->FindRow<FWuReLocalizationStruct>(RowName, ContextString))
		{
			return *Row;
		}
	}
	return FWuReLocalizationStruct(FText::FromName(RowName), FText::FromName(RowName));
}
