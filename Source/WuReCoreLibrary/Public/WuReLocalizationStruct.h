// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "Engine/DataTable.h"
#include "WuReLocalizationStruct.generated.h"

USTRUCT(BlueprintType)
struct FWuReLocalizationStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Context", meta=(MultiLine="true"))
	FText ContextKoKr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Context", meta=(MultiLine="true"))
	FText ContextEnGb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Context")
	TObjectPtr<USoundBase> Voiceline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Context")
	float ManualDuration;

	FWuReLocalizationStruct() : ContextKoKr(FText::FromString(TEXT("한글 컨텍스트"))), ContextEnGb(FText::FromString(TEXT("Global English Context"))), Voiceline(nullptr), ManualDuration(0.f) {};
	FWuReLocalizationStruct(FText KoKr, FText EnGb)
	{
		ContextKoKr = KoKr;
		ContextEnGb = EnGb;
		Voiceline = nullptr;
		ManualDuration = 0.f;
	}
};
