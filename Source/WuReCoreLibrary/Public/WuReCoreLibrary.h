// © Platina Production 2017~2024

#pragma once

#include "EngineMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(WuReCoreLibrary, Log, All);

#define WURELOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define WURELOG_S(Verbosity) UE_LOG(WuReCoreLibrary, Verbosity, TEXT("%s"), *WURELOG_CALLINFO)
#define WURELOG(Verbosity, Format, ...) UE_LOG(WuReCoreLibrary, Verbosity, TEXT("%s: %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define WURELOG_DISPLAY(Duration, ColorHex, Format, ...) GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::FromHex(ColorHex), *FString::Printf(Format, ##__VA_ARGS__));
#define WURELOG_DISPLAY_S(Duration, ColorHex, Format, ...) \
GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::FromHex(ColorHex), *FString::Printf(TEXT("%s: %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__)));

class FWuReCoreLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
