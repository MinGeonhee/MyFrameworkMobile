# MyFrameworkMobile

DirectX11 기반 자체 엔진을 모바일 환경까지 확장하기 위한 실험/검증 프로젝트입니다.

기존 Windows 중심 엔진 구조를 최대한 유지하면서 Diligent Engine을 통해 렌더링 API 차이를 감싸고, Android에서는 Vulkan 기반으로 APK 실행을 확인하는 것을 목표로 합니다.

## 현재 목표

- 기존 Engine / Client 구조와 Engine DLL 방식을 유지
- Windows Client와 Android 실행부가 공통 Game 프로젝트를 공유
- Android Gradle / NDK / CMake 기반 APK 빌드 환경 구성
- Windows와 Android 양쪽에서 동일한 테스트 씬 렌더링 확인
- 향후 모바일 콘텐츠 개발을 이어갈 수 있는 최소 구조 확보

## 프로젝트 구조

```text
C:\MyFrameworkMobile
├─ Engine        엔진 공통 코드. 렌더링, 리소스, 매니저, 컴포넌트 관리
├─ Game          Windows / Android가 함께 사용하는 게임 로직 프로젝트
├─ Client        Windows 실행 프로젝트. WinAPI 창 생성과 실행 루프 담당
├─ Android       Android 실행 프로젝트. Gradle / NDK / NativeActivity / APK 빌드 담당
├─ EngineSDK     Client와 Game에서 참조할 Engine 공개 헤더와 라이브러리 복사 위치
├─ Library       외부 라이브러리. Diligent Engine 포함
└─ Resource      테스트 이미지, 디버그 리소스, Android assets 복사 대상
```

## 어디서 코드를 작성하는가

### 게임/콘텐츠 코드

몬스터, 플레이어, 테스트 오브젝트, 씬, 게임 로직은 `Game` 프로젝트에 작성합니다.

```text
C:\MyFrameworkMobile\Game
```

Windows Client와 Android 실행부가 같은 Game 코드를 바라보는 구조이므로, 일반적인 콘텐츠 작업은 우선 Game에서 진행합니다.

### 엔진 코드

ResourceManager, Shader, Texture, Model, GameObject, Component, Renderer 같은 엔진 기능은 `Engine` 프로젝트에 작성합니다.

```text
C:\MyFrameworkMobile\Engine
```

그래픽 디바이스 초기화, SwapChain, Present, Diligent / Vulkan 연동처럼 플랫폼과 렌더링 API에 가까운 처리는 Client가 아니라 Engine 내부에 두는 것을 원칙으로 합니다.

### Windows 실행 코드

Windows 창 생성, 메시지 루프, Windows 전용 실행 진입점은 `Client` 프로젝트에 둡니다.

```text
C:\MyFrameworkMobile\Client
```

Client는 실행 껍데기 역할만 담당하고, 실제 게임 로직은 Game으로 분리합니다.

### Android 실행 코드

Android Studio에서 열 프로젝트는 아래 폴더입니다.

```text
C:\MyFrameworkMobile\Android
```

Android 폴더는 APK 빌드, NativeActivity, ANativeWindow, Android 이벤트 연결을 담당합니다. 게임 로직을 Android 폴더에 중복 작성하지 않는 것을 원칙으로 합니다.

## 렌더링 구조

```text
Game / Client / Android
        ↓
Engine
        ↓
Diligent Engine
        ↓
Windows: DirectX11 또는 Vulkan
Android: Vulkan
```

Diligent Engine은 게임 엔진 전체가 아니라 그래픽 API 차이를 감싸는 렌더링 추상화 계층으로 사용합니다. Scene, GameObject, ResourceManager 같은 엔진 시스템은 MyFrameworkMobile에서 직접 관리합니다.

## Windows 빌드

Visual Studio에서 아래 솔루션을 엽니다.

```text
C:\MyFrameworkMobile\MyFrameworkMobile.sln
```

권장 구성:

```text
Configuration: Debug 또는 Release
Platform: x64
```

빌드 결과는 각 프로젝트의 `Bin\Debug`, `Bin\Release` 아래에 생성됩니다.

## Android 빌드

Android Studio에서 아래 폴더를 엽니다.

```text
C:\MyFrameworkMobile\Android
```

또는 PowerShell에서 다음 명령을 실행합니다.

```powershell
cd C:\MyFrameworkMobile\Android
.\gradlew.bat :app:assembleDebug
```

Debug APK 결과물:

```text
C:\MyFrameworkMobile\Android\app\build\outputs\apk\debug\app-debug.apk
```

현재 ABI는 에뮬레이터와 실기기 테스트를 위해 `x86_64`, `arm64-v8a`를 함께 고려합니다.

## 현재 검증 상태

- Windows Client 실행 화면 출력 확인
- Android APK 빌드 확인
- Android 실기기 실행 확인
- Diligent / Vulkan 기반 렌더링 출력 확인
- FPS 디버그 출력 확인
- Windows / Android 공통 Game 구조 기반 실행 확인

## 작업 원칙

- 기존 프레임워크 스타일과 폴더 구조를 최대한 유지합니다.
- Platform별 실행부는 얇게 유지하고, 공통 게임 로직은 Game 프로젝트로 모읍니다.
- 그래픽 API별 차이는 Engine 내부에서 감싸고, Client / Game으로 새지 않게 합니다.
- Resource, Bin, Lib, 빌드 산출물은 Git에 올리지 않습니다.
