# MyFramework Mobile - Client

Client는 운영체제 창을 생성하고 MyFramework Engine을 실행하는 애플리케이션 프로젝트다.

현재 Client에는 게임 콘텐츠가 없으며 다음 역할만 담당한다.

- WinAPI 창 생성
- Windows 메시지 루프
- 그래픽 API 선택
- Engine 초기화
- Update와 Render 호출
- 창 크기 변경을 Engine에 전달

Device, DeviceContext, SwapChain, Clear, Resize, Present 등의 그래픽 처리는 Engine이 담당한다.

## 솔루션 열기

프로젝트 루트의 솔루션을 Visual Studio에서 연다.

```text
MyFrameworkMobile/MyFrameworkMobile.sln
```

빌드 구성은 다음을 사용한다.

```text
Configuration: Debug
Platform: x64
```

솔루션을 빌드하면 Engine이 먼저 빌드되고 필요한 공개 헤더와 라이브러리가 `EngineSDK`로 갱신된 후 Client가 빌드된다.

## 그래픽 API 선택

`Client/Default/Client.cpp`에서 다음 한 줄로 사용할 그래픽 API를 선택한다.

Vulkan:

```cpp
constexpr GRAPHIC_API GRAPHIC_BACKEND = GRAPHIC_API::VULKAN;
```

DirectX 11:

```cpp
constexpr GRAPHIC_API GRAPHIC_BACKEND = GRAPHIC_API::D3D11;
```

모바일 확장을 고려한 기본값은 Vulkan이다.

Client는 선택값만 Engine에 전달한다. 실제 Vulkan 또는 DirectX 11 초기화는 `Engine/Private/CGraphicDevice.cpp`에서 수행된다.

## 실행

Visual Studio에서 Client 프로젝트를 시작 프로젝트로 지정한 뒤 실행하거나 다음 파일을 실행한다.

```text
Client/Bin/Client.exe
```

필요한 DLL은 Engine 빌드 후 `Client/Bin`에 자동으로 복사된다.

Debug Vulkan 실행 시:

```text
Client.exe
Engine.dll
GraphicsEngineVk_64d.dll
```

Debug DirectX 11 실행 시:

```text
Client.exe
Engine.dll
GraphicsEngineD3D11_64d.dll
```

## 현재 실행 흐름

```text
wWinMain
    → WinAPI 창 생성
    → CMainApp::Create
    → CGameManager::Initialize_Engine

Message Loop
    → CMainApp::Update
    → CMainApp::Render

WM_SIZE
    → CMainApp::Resize
    → Engine의 SwapChain Resize
```

`ESC` 키를 누르면 Client 창이 종료된다.

## Client 작성 원칙

- Client에서 Diligent 헤더를 직접 포함하지 않는다.
- Client에서 `Present()`를 호출하지 않는다.
- 그래픽 Device나 SwapChain을 Client 멤버로 보관하지 않는다.
- Client는 사용할 그래픽 API와 창 정보만 Engine에 전달한다.
- 게임 콘텐츠가 추가되면 `CMainApp`에서 초기 Scene과 Client 전용 시스템을 준비한다.

