# MyFramework Mobile - Engine

`MyFrameworkMobile/Engine`은 기존 DirectX 11 기반 MyFramework의 구조와 스타일을 유지하면서 Windows와 Android를 함께 지원하기 위한 엔진 프로젝트다.

Engine은 DLL로 빌드되며 Client는 `EngineSDK/Inc`와 `EngineSDK/Lib`를 통해 Engine의 공개 인터페이스를 사용한다. 그래픽 API별 차이는 Engine 내부의 `CGraphicDevice`에서 처리하고 Client나 상위 Manager에는 노출하지 않는 것을 원칙으로 한다.

## Engine의 현재 책임

- `Engine.dll`, `Engine.lib` 생성
- 공개 헤더를 `EngineSDK/Inc`로 갱신
- `CGameManager`를 통한 엔진 시스템 접근
- `CTimeManger`, `CTimer`를 통한 프레임 시간 관리
- 그래픽 API 선택에 따른 Device, Context, SwapChain 초기화
- RenderTarget과 DepthStencil 설정 및 Clear
- SwapChain Resize
- 최종 Present
- 이후 `ResourceManager`, `Shader`, `Texture`, `Model`, `Collider` 등의 기존 시스템 이전

## Diligent Engine을 사용하는 이유

Diligent Engine은 완성된 게임 엔진이 아니라 DirectX 11, DirectX 12, Vulkan 등의 저수준 그래픽 API를 공통 인터페이스로 감싸는 그래픽 추상화 계층이다.

```text
MyFramework의 Engine 시스템
        ↓
CGraphicDevice 및 Engine 내부 래퍼
        ↓
Diligent 공통 그래픽 인터페이스
        ↓
Vulkan 또는 DirectX 11 백엔드
        ↓
그래픽 드라이버와 GPU
```

Diligent가 `ResourceManager`, `SceneManager`, `Model`, `Collider` 같은 게임 엔진 시스템을 대신 만들어 주는 것은 아니다. 해당 시스템은 MyFramework가 직접 관리하고, 그 내부에서 사용되는 그래픽 API 의존 코드만 Diligent 인터페이스로 작성한다.

기존 DirectX 11에서 직접 처리했던 다음 작업의 API별 차이를 Diligent가 감싸준다.

- Device와 DeviceContext 생성
- DXGI 또는 Vulkan SwapChain 생성
- RenderTarget과 DepthStencil 관리
- Buffer와 Texture 생성
- Shader 및 PipelineState 생성
- Descriptor와 Shader Resource 연결
- 화면 크기 변경과 SwapChain 재생성
- 리소스 상태 전환
- Draw, DrawIndexed, Dispatch, Present 호출

## 그래픽 API 선택 흐름

사용할 그래픽 API는 Client가 `GRAPHIC_API` 값으로 선택하지만, 실제 초기화는 모두 Engine에서 수행한다.

```cpp
enum class GRAPHIC_API
{
    D3D11,
    VULKAN
};
```

Client가 `Engine_Desc`를 작성해 `CGameManager::Initialize_Engine()`에 전달하면 `CGraphicDevice`가 선택값을 확인한다.

```text
Engine_Desc
 ├─ eGraphicAPI
 ├─ pNativeWindow
 ├─ iWinSizeX
 └─ iWinSizeY
        ↓
CGameManager::Initialize_Engine
        ↓
CGraphicDevice::Initialize
        ↓
GRAPHIC_API::VULKAN → Initialize_Vulkan
GRAPHIC_API::D3D11  → Initialize_DirectX11
```

### Vulkan 선택

```text
GraphicsEngineVk 백엔드 로드
    → IEngineFactoryVk 획득
    → Vulkan Device와 DeviceContext 생성
    → Native Window를 이용해 Surface 및 SwapChain 생성
```

Diligent 내부에서는 Vulkan Instance, Physical Device, Logical Device, Queue, Surface, SwapChain과 같은 Vulkan 초기화 과정을 처리한다.

### DirectX 11 선택

```text
GraphicsEngineD3D11 백엔드 로드
    → IEngineFactoryD3D11 획득
    → D3D11 Device와 DeviceContext 생성
    → Win32 Native Window를 이용해 DXGI SwapChain 생성
```

현재 Windows Debug x64에서는 Vulkan과 DirectX 11 백엔드가 모두 빌드되고 실제 실행까지 확인된 상태다. Android에서는 Vulkan 백엔드만 사용할 예정이다.

## 렌더링 흐름

Client는 엔진에 Render를 요청할 뿐 SwapChain이나 Present를 직접 다루지 않는다.

```text
CMainApp::Render
    → CGameManager::Draw
        → CGraphicDevice::Begin_Render
            → RenderTarget 설정
            → Color Clear
            → DepthStencil Clear

        → RenderManager 등 실제 렌더링 시스템

        → CGraphicDevice::End_Render
            → ISwapChain::Present
```

향후 `RenderManager`를 이전하면 실제 오브젝트 렌더링은 `Begin_Render()`과 `End_Render()` 사이에서 수행한다.

## Resize 흐름

Windows의 `WM_SIZE` 또는 Android의 Surface 크기 변경 이벤트는 Client가 받는다. Client는 새 크기만 Engine에 전달한다.

```text
운영체제 창 크기 변경
    → CMainApp::Resize
    → CGameManager::Resize_Engine
    → CGraphicDevice::Resize
    → ISwapChain::Resize
```

백버퍼 크기가 0인 최소화 상태에서는 Resize를 실행하지 않는다.

## 주요 Diligent 인터페이스와 DirectX 11 대응 관계

| Diligent 인터페이스 | DirectX 11 대응 객체 | 주 역할 |
|---|---|---|
| `IRenderDevice` | `ID3D11Device` | GPU 리소스 생성 |
| `IDeviceContext` | `ID3D11DeviceContext` | 렌더링 명령 실행 |
| `ISwapChain` | `IDXGISwapChain` | 백버퍼, Resize, Present |
| `IBuffer` | `ID3D11Buffer` | Vertex, Index, Constant 등의 버퍼 |
| `ITexture` | `ID3D11Texture1D/2D/3D` | 이미지와 렌더 타깃 리소스 |
| `ITextureView` | `SRV`, `RTV`, `DSV`, `UAV` | 텍스처 접근 방법 정의 |
| `IShader` | `ID3D11VertexShader` 등 | GPU 셰이더 프로그램 |
| `IPipelineState` | Shader와 여러 D3D11 State의 조합 | 렌더링 파이프라인 상태 묶음 |

## IRenderDevice

DirectX 11의 `ID3D11Device`에 해당한다. GPU 리소스를 생성하는 공장 역할을 하며 직접 Draw 명령을 실행하지는 않는다.

주요 생성 대상:

- `IBuffer`
- `ITexture`
- `ISampler`
- `IShader`
- `IPipelineState`
- `IPipelineResourceSignature`
- `IFence`, `IQuery` 등의 동기화 및 조회 객체

DirectX 11에서 다음과 같이 생성하던 리소스가 `IRenderDevice` 기반 코드로 바뀐다.

```cpp
ID3D11Device::CreateBuffer(...);
ID3D11Device::CreateTexture2D(...);
ID3D11Device::CreateVertexShader(...);
ID3D11Device::CreatePixelShader(...);
ID3D11Device::CreateSamplerState(...);
```

## IDeviceContext

DirectX 11의 `ID3D11DeviceContext`에 해당한다. 생성된 리소스와 PipelineState를 연결하고 실제 GPU 명령을 실행한다.

주요 역할:

- Vertex Buffer와 Index Buffer 연결
- PipelineState 설정
- Shader Resource와 Constant Buffer 연결
- RenderTarget 설정 및 Clear
- `Draw`, `DrawIndexed`, `Dispatch` 실행
- Buffer와 Texture 데이터 갱신
- 리소스 상태 전환

DirectX 11에서는 다음과 같이 여러 단계로 설정했다.

```cpp
pContext->IASetVertexBuffers(...);
pContext->IASetIndexBuffer(...);
pContext->VSSetShader(...);
pContext->PSSetShader(...);
pContext->DrawIndexed(...);
```

Vulkan에서는 Command Buffer, Command Pool, Queue 등을 직접 다뤄야 하지만 Diligent가 이를 `IDeviceContext` 형태로 감싼다.

## ISwapChain

DirectX 11의 `IDXGISwapChain`에 해당한다. 화면에 표시할 백버퍼와 기본 깊이 버퍼를 관리한다.

주요 역할:

- 현재 백버퍼의 RenderTargetView 제공
- 기본 DepthStencilView 제공
- 창 크기 변경 시 SwapChain Resize
- 완성된 백버퍼를 화면에 Present

```cpp
ITextureView* pBackBuffer = pSwapChain->GetCurrentBackBufferRTV();
ITextureView* pDepthBuffer = pSwapChain->GetDepthBufferDSV();

pSwapChain->Resize(Width, Height);
pSwapChain->Present(1);
```

현재 프로젝트에서는 `CGraphicDevice`만 `ISwapChain`을 소유한다.

## IBuffer

DirectX 11의 `ID3D11Buffer`에 해당한다. 하나의 `IBuffer` 인터페이스가 생성 정보와 Bind Flag에 따라 여러 용도로 사용된다.

### Vertex Buffer

정점 데이터를 저장한다.

- Position
- Normal
- TexCoord
- Tangent
- Vertex Color
- Bone Index와 Bone Weight

DirectX 11의 `D3D11_BIND_VERTEX_BUFFER`에 대응하며 Diligent에서는 `BIND_VERTEX_BUFFER`를 사용한다.

### Index Buffer

Vertex Buffer의 정점을 어떤 순서로 조합할지 나타내는 인덱스를 저장한다. 동일한 정점을 여러 삼각형에서 공유해 메모리와 정점 처리 중복을 줄인다.

```text
Vertex Buffer: [정점0, 정점1, 정점2, 정점3]
Index Buffer : [0, 1, 2, 0, 2, 3]
```

위 예시는 정점 네 개를 이용해 삼각형 두 개로 사각형을 만든다. DirectX 11의 `D3D11_BIND_INDEX_BUFFER`에 대응하며 Diligent에서는 `BIND_INDEX_BUFFER`를 사용한다.

### Constant Buffer

셰이더가 공통으로 읽는 작은 상수 데이터를 저장한다.

- World, View, Projection 행렬
- 카메라 위치
- Material 색상과 속성
- Light 정보
- 시간과 화면 크기

DirectX 11의 Constant Buffer와 같은 개념이며 Diligent에서는 일반적으로 `BIND_UNIFORM_BUFFER`를 사용한다. Vulkan에서는 Uniform Buffer에 대응한다.

### Structured Buffer

동일한 구조체가 배열 형태로 연속 저장된 버퍼다. 셰이더에서 구조체 배열처럼 접근한다.

```cpp
struct LightData
{
    float4 vPosition;
    float4 vColor;
};
```

다수의 Light, Instance Transform, Bone Matrix, Particle 데이터를 전달할 때 유용하다. 각 원소의 크기인 `ElementByteStride`를 지정해야 한다.

읽기 전용이면 Shader Resource로 사용하고 GPU 쓰기도 필요하면 Unordered Access를 함께 사용한다.

### Storage Buffer

Vulkan과 범용 GPU 용어에서 사용하는 이름으로, 셰이더가 큰 데이터를 읽거나 쓸 수 있는 버퍼다. DirectX 11에서는 보통 Structured Buffer 또는 Raw Buffer와 UAV 조합으로 대응한다.

주요 용도:

- Compute Shader 결과 저장
- GPU Particle 상태 갱신
- GPU Culling 결과 저장
- 대량 Instance 데이터 처리
- 셰이더 사이의 공유 데이터

Constant Buffer보다 큰 데이터를 다룰 수 있고 GPU 쓰기가 가능하지만 리소스 상태와 동기화에 주의해야 한다.

### Indirect Argument Buffer

CPU가 Draw 인수를 직접 지정하지 않고 GPU가 생성한 인수를 사용해 Draw 또는 Dispatch를 실행하기 위한 버퍼다.

DirectX 11의 다음 기능과 대응한다.

```cpp
ID3D11DeviceContext::DrawInstancedIndirect(...);
ID3D11DeviceContext::DrawIndexedInstancedIndirect(...);
ID3D11DeviceContext::DispatchIndirect(...);
```

대표적인 사용 흐름:

```text
Compute Shader가 보이는 오브젝트 수 계산
    → Indirect Argument Buffer에 Draw 인수 기록
    → CPU로 결과를 다시 읽지 않음
    → GPU가 해당 인수로 DrawIndirect 실행
```

GPU Driven Rendering, GPU Culling, 대규모 Instance Rendering에 사용한다. 초기 단계에서는 일반 Vertex, Index, Constant Buffer를 먼저 구현한 뒤 추가한다.

## ITexture와 ITextureView

`ITexture`는 실제 텍스처 메모리 리소스이고 `ITextureView`는 해당 텍스처를 어떤 목적으로 접근할지 정의한다.

| Diligent Texture View | DirectX 11 대응 |
|---|---|
| `TEXTURE_VIEW_SHADER_RESOURCE` | `ID3D11ShaderResourceView` |
| `TEXTURE_VIEW_RENDER_TARGET` | `ID3D11RenderTargetView` |
| `TEXTURE_VIEW_DEPTH_STENCIL` | `ID3D11DepthStencilView` |
| `TEXTURE_VIEW_UNORDERED_ACCESS` | `ID3D11UnorderedAccessView` |

하나의 Texture가 생성 옵션에 따라 Shader Resource와 RenderTarget 등 여러 View를 가질 수 있다.

기존 `CTexture`는 내부에서 `ITexture`와 필요한 `ITextureView`를 보관하는 방향으로 이전한다.

## IShader

DirectX 11의 다음 셰이더 객체들을 공통 인터페이스 하나로 표현한다.

- `ID3D11VertexShader`
- `ID3D11PixelShader`
- `ID3D11GeometryShader`
- `ID3D11HullShader`
- `ID3D11DomainShader`
- `ID3D11ComputeShader`

생성할 때 `SHADER_TYPE_VERTEX`, `SHADER_TYPE_PIXEL` 등의 타입을 지정한다.

기존 엔진의 `CShader`는 여러 `IShader`, `IPipelineState`, Shader Resource Binding을 관리하는 방향으로 이전할 수 있다.

## IPipelineState

DirectX 11에서는 렌더링 상태를 여러 객체와 함수 호출로 따로 설정했다.

```cpp
pContext->IASetInputLayout(...);
pContext->IASetPrimitiveTopology(...);
pContext->VSSetShader(...);
pContext->PSSetShader(...);
pContext->RSSetState(...);
pContext->OMSetBlendState(...);
pContext->OMSetDepthStencilState(...);
```

Diligent에서는 다음 정보를 `IPipelineState` 중심으로 묶는다.

- Vertex, Pixel 및 기타 Shader
- Input Layout
- Primitive Topology
- Rasterizer State
- Blend State
- DepthStencil State
- RenderTarget Format
- Depth Buffer Format

사용할 때는 Context에 다음과 같이 설정한다.

```cpp
pDeviceContext->SetPipelineState(pPipelineState);
```

Vulkan은 많은 파이프라인 상태를 미리 묶어 생성하므로 Diligent도 Vulkan에 자연스럽게 대응할 수 있도록 PipelineState 중심 구조를 사용한다.

## 기존 엔진 클래스의 이전 방향

```text
CGraphicDevice
 ├─ IRenderDevice
 ├─ IDeviceContext
 └─ ISwapChain

CVIBuffer / CMesh
 ├─ IBuffer (Vertex Buffer)
 └─ IBuffer (Index Buffer)

CTexture
 ├─ ITexture
 └─ ITextureView

CShader
 ├─ IShader
 ├─ IPipelineState
 └─ Shader Resource Binding
```

상위 클래스 이름과 Manager 구조는 기존 MyFramework 스타일을 유지하고 내부의 DirectX 11 전용 포인터와 생성 코드만 Diligent 인터페이스로 교체한다.

## Diligent 타입 노출 원칙

Diligent는 Engine의 구현 세부 사항으로 취급한다.

- Client 공개 인터페이스에 Diligent 헤더를 포함하지 않는다.
- Client는 Diligent Device, Context, SwapChain을 보관하지 않는다.
- `Present()`는 Engine에서만 호출한다.
- 상위 Manager에는 가능하면 MyFramework의 타입과 래퍼를 노출한다.
- Diligent 교체가 필요할 경우 `CGraphicDevice` 이하의 변경으로 제한한다.

이 원칙을 지키면 향후 Diligent를 직접 Vulkan 구현으로 교체해도 Client와 게임 로직의 변경을 최소화할 수 있다.

## Engine 빌드 결과

Debug x64 기준 주요 결과물:

```text
Engine/Bin/Engine.dll
Engine/Bin/Engine.lib
EngineSDK/Inc/*.h
EngineSDK/Lib/Engine.lib
Client/Bin/Engine.dll
Client/Bin/GraphicsEngineVk_64d.dll
Client/Bin/GraphicsEngineD3D11_64d.dll
```

Engine 빌드 후 공개 헤더, import library, DLL, Diligent 백엔드 DLL이 필요한 위치로 복사된다.

## 현재 상태

- Windows Debug x64 빌드 성공
- Engine DLL 링크 방식 유지
- Vulkan 백엔드 실제 실행 확인
- DirectX 11 백엔드 실제 실행 확인
- Device, Context, SwapChain을 Engine에서 소유
- Resize, Clear, Present를 Engine에서 처리
- `CTimeManger`, `CTimer` 이전 완료
- Android Native Window 및 APK 빌드 구성은 이후 추가 예정

