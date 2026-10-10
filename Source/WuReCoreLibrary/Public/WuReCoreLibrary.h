// © Platina Production 2017~2024

#pragma once

#include "EngineMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWuReCoreLibrary, Log, All);

// #define WURELOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
// #define WURELOG_S(Verbosity) UE_LOG(WuReCoreLibrary, Verbosity, TEXT("%s"), *WURELOG_CALLINFO)
// #define WURELOG(Verbosity, Format, ...) UE_LOG(WuReCoreLibrary, Verbosity, TEXT("%s: %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
// #define WURELOG_DISPLAY(Duration, ColorHex, Format, ...) GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::FromHex(ColorHex), *FString::Printf(Format, ##__VA_ARGS__));
// #define WURELOG_DISPLAY_S(Duration, ColorHex, Format, ...) \
// GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::FromHex(ColorHex), *FString::Printf(TEXT("%s: %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__)));

#define WURELOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT("):"))

#define WURELOG_S(Verbosity) \
    UE_LOG(LogWuReCoreLibrary, Verbosity, TEXT("%s"), *WURELOG_CALLINFO)

#define WURELOG(Verbosity, Format, ...) \
    UE_LOG(LogWuReCoreLibrary, Verbosity, TEXT("%s %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// __FUNCTION__ is compile-time (the C++ base class); this adds the runtime class (e.g. WBP_Foo_C) and instance name.
#define WURELOG_OBJ(Verbosity, Object, Format, ...) \
    UE_LOG(LogWuReCoreLibrary, Verbosity, TEXT("[%s / %s] %s %s"), *GetNameSafe((Object)->GetClass()), *GetNameSafe(Object), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define WURELOG_DISPLAY(Duration, ColorHex, Format, ...) \
    do { \
        if (GEngine) \
        { \
            GEngine->AddOnScreenDebugMessage(-1, (Duration), FColor::FromHex(ColorHex), *FString::Printf(Format, ##__VA_ARGS__)); \
        } \
    } while(0)

#define WURELOG_DISPLAY_S(Duration, ColorHex, Format, ...) \
    do { \
        if (GEngine) \
        { \
            GEngine->AddOnScreenDebugMessage(-1, (Duration), FColor::FromHex(ColorHex), *FString::Printf(TEXT("%s %s"), *WURELOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))); \
        } \
    } while(0)

class FWuReCoreLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
