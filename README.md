# WU:RE Core Library
WU:RE Core Library는 WU:RE 게임 프로젝트들이 같이 쓰는 Unreal Engine 5.8 런타임 C++ 플러그인입니다.


> [Bitmap Developer](https://developer.prodbybitmap.com/ko/unreal-study/wurecorelibrary)를 참고하십시오.
> 
> Read [Bitmap Developer](README_EnGb.md) article for detailed English description.

# 주요 기능
* 비동기 레벨 스트리밍
* 설정 저장과 그래픽 (DLSS)
* CommonUI 기반 UI 프레임워크
* [Froyok](https://www.froyok.fr/blog/2019-03-creating-seamless-portals-in-unreal-engine-4/) 포털 시스템
* 자체 로그 및 매크로

# 요구 사항
1. [NVIDIA DLSS](https://developer.nvidia.com/rtx/dlss/get-started) Unreal Engine Plugin
2. [Unreal Engine C++ 프로그래밍을 위한 준비 사항](https://developer.prodbybitmap.com/ko/unreal-study/prerequisites)

# 설치 및 사용 방법
1. 최신 버전의 플러그인을 [다운로드](#)합니다.
2. 프로젝트의 플러그인 폴더에 WuReCoreLibrary 폴더를 옮깁니다. 이 때, 엔진의 플러그인 폴더에 탑재하지 않도록 하세요.
3. `The Following modules are mission or built with a different engine version: WuReCoreLibrary. Would you like to rebuild them now?` 팝업이 발생한다면 Yes를 클릭하여 플러그인 모듈을 다시 빌드합니다.
4. 에디터에서 프로젝트에 대해 WU:RE Core Library 플러그인을 활성화합니다.
5. C++ 개발 환경에 사용할 목적인 경우, `YOURPROJECTNAME.Build.cs`에서 `PublicDependencyModuleNames`에 `WuReCoreLibrary`를 추가합니다.
```csharp
// ...
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", /* ... */ ,"WuReCoreLibrary" });
// ...
```
