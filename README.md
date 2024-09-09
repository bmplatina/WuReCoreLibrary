# WU:RE Core Library
WU:RE에서 제작하는 게임을 위한 Unreal Engine 코어 라이브러리입니다.

Read [this](README_EnGb.md) for English Description.

# 요구 사항
1. [NVIDIA DLSS](https://developer.nvidia.com/rtx/dlss/get-started) Unreal Engine Plugin
2. [Unreal Engine C++ 프로그래밍을 위한 준비 사항](https://developer.prodbybitmap.com/ko/unreal-study/prerequisites)

# 설치 및 사용 방법
1. 최신 버전의 플러그인을 [다운로드](#)합니다.
2. 프로젝트의 플러그인 폴더에 WuReCoreLibrary 폴더를 옮깁니다. 이 때, 엔진의 플러그인 폴더에 탑재하지 않도록 하세요.
3. `The Following modules are mission or built with a different engine version: WuReCoreLibrary. Would you like to rebuild them now?` 팝업이 발생한다면 Yes를 클릭하여 플러그인 모듈을 다시 빌드합니다.
4. 에디터에서 프로젝트에 대해 WU:RE Core Library 플러그인을 활성화합니다.
5. `YOURPROJECTNAME.Build.cs`에서 `PublicDependencyModuleNames`에 `WuReCoreLibrary`를 추가합니다.
```csharp
// ...
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", /* ... */ ,"WuReCoreLibrary" });
// ...
```

# 기능
클래스를 상속받아 프로젝트에서 사용하세요.

## `class UWuReGameInstance: public UGameInstance`
* Steam 온라인 서브시스템 연동 기능
  * AppID (`FString SteamAppID`)의 값을 Steamworks의 앱 ID로 변경하세요.
* 설정 저장 기능
  * 사운드 클래스 믹스를 통하여 볼륨을 저장합니다.
  * 그래픽 설정을 저장합니다.
  * `virtual void Init() override`, `FSettings`, `FDLSSUserSettings` 프로퍼티를 참조하세요.

## `class AWuRePlayerController: public APlayerController`

## `struct FWuReLocalizationStruct: public FTableRowBase`
* 한글/영어 현지화 지원
  * `FText ContextKoKr`, `FText ContextEnGb`, `FSettings` 를 참조하세요.
* 필요한 경우, 보이스라인 사운드 베이스 또는 자막의 표시 시간 조정 기능
  * `TObjectPtr<USoundBase> Voiceline`, `float ManualDuration` 프로퍼티를 참조하세요.

## `class UWuReUserWidget: public UUserWidget`
* 게임패드 지원

## `class UWuReWidgetComponent: public UWuReUserWidget`
* 컴포넌트로서 사용되는 위젯