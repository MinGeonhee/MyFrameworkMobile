# Engine

MyFrameworkMobile의 공통 엔진 프로젝트입니다.

기존 DirectX11 기반 MyFramework의 구조와 스타일을 최대한 유지하면서, Windows와 Android를 함께 지원할 수 있도록 Diligent Engine 기반 렌더링 추상화 구조를 추가했습니다.

## 역할

- Engine DLL / LIB 빌드
- 공개 헤더와 import library를 EngineSDK로 갱신
- GameObject, Component, ResourceManager, ShaderManager 등 엔진 시스템 관리
- Diligent 기반 RenderDevice, DeviceContext, SwapChain 관리
- Windows / Android 렌더링 초기화 차이를 Engine 내부에서 캡슐화
- Render Begin / End / Present 처리
- Resize, Viewport, RenderTarget, DepthStencil 관리

## Diligent Engine 사용 이유

Diligent Engine은 완성형 게임 엔진이 아니라, DirectX11 / Vulkan / Metal 같은 그래픽 API 차이를 공통 인터페이스로 감싸주는 렌더링 추상화 계층입니다.

이 프로젝트에서는 다음처럼 사용합니다.

```text
MyFrameworkMobile Engine
        ↓
CGraphicDevice / Render 관련 래퍼
        ↓
Diligent Engine 공통 인터페이스
        ↓
Windows: DirectX11 또는 Vulkan
Android: Vulkan
```

Scene, GameObject, Component, ResourceManager 같은 시스템은 Diligent가 대신해주는 것이 아니라 기존 엔진 구조를 유지하며 직접 관리합니다.

## 주요 Diligent 인터페이스 대응

| Diligent | DirectX11 기준 | 역할 |
|---|---|---|
| IRenderDevice | ID3D11Device | GPU 리소스 생성 |
| IDeviceContext | ID3D11DeviceContext | 렌더링 명령 실행 |
| ISwapChain | IDXGISwapChain | BackBuffer, Resize, Present |
| IBuffer | ID3D11Buffer | Vertex / Index / Constant Buffer |
| ITexture | ID3D11Texture2D | 텍스처 리소스 |
| ITextureView | SRV / RTV / DSV / UAV | 텍스처 접근 방식 |
| IShader | VertexShader / PixelShader 등 | GPU shader program |
| IPipelineState | Shader + Render State 묶음 | 렌더링 파이프라인 상태 |

## 렌더링 책임 분리

Client나 Android 실행부는 그래픽 디바이스를 직접 들고 있지 않는 것을 원칙으로 합니다.

```text
Client / Android
    → 실행 창, NativeWindow, 입력, 앱 생명주기 전달
Engine
    → Device 생성, SwapChain 관리, RenderTarget 설정, Present 처리
Game
    → 실제 게임 오브젝트와 씬 구성
```

## 빌드 산출물

```text
Engine/Bin/Debug/Engine.dll
Engine/Bin/Release/Engine.dll
EngineSDK/Lib/Debug/Engine.lib
EngineSDK/Lib/Release/Engine.lib
EngineSDK/Public 또는 EngineSDK/Inc 공개 헤더
```

Bin, Lib, 빌드 중간 산출물은 Git 관리 대상이 아닙니다.
