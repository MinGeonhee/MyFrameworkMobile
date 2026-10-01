#pragma once

#include "CBase.h"

NS_BEGIN(Engine)

class CShader;

/* Draw 시점에 PSO 를 고르기 위해 필요한 현재 상태 */
struct GRAPHIC_PIPELINE_STATE
{
    RASTERIZER_DESC         tRasterizer{};
    DEPTH_STENCIL_DESC      tDepthStencil{};
    BLEND_DESC              tBlend{};
    uint32                  iStencilRef{ 0 };
    float                   fBlendFactor[4]{ 0.f,0.f,0.f,0.f };

    uint32                  iNumRenderTargets{ 0 };
    TEXTURE_FORMAT          eRTVFormats[8]{};
    TEXTURE_FORMAT          eDSVFormat{ Diligent::TEX_FORMAT_UNKNOWN };

    PRIMITIVE_TOPOLOGY      eTopology{ Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
    uint32                  iVertexStrides[2]{ 0,0 };
};

/* 원본 : ID3D11Device / ID3D11DeviceContext / IDXGISwapChain 을 직접 생성했다.                         */
/* 현재 : Diligent 의 IRenderDevice / IDeviceContext / ISwapChain 을 생성한다. (Windows D3D11·Vulkan , Android Vulkan) */
/*                                                                                                        */
/* DX11 은 OMSetBlendState , RSSetState , IASetInputLayout 처럼 상태를 하나씩 바꾸는 상태머신이지만             */
/* Vulkan(Diligent) 은 Shader + RenderState + RenderTarget Format 을 하나로 묶은 PSO 를 사용한다.               */
/* 엔진 상위 코드(Effect 스타일 Apply_Pass , RenderStateManager)를 그대로 쓰기 위해                             */
/* GraphicDevice 가 DX11 처럼 "현재 상태" 를 기억하고 있다가 Draw 시점에 PSO 를 선택한다.                        */

class CGraphicDevice final : public CBase
{
public:
    using PIPELINE_STATE = GRAPHIC_PIPELINE_STATE;

private:
    CGraphicDevice();
    virtual ~CGraphicDevice() = default;
public:
    /* 그래픽 디바이스의 초기화. */
    /* 장치객체를 생성한다. */
    HRESULT Initialize( const GraphicDevice_Desc& tDesc,ComPtr<Device>* pDevice, ComPtr<DeviceContext>* pContext);

    /* 백버퍼를 지운다. */
    HRESULT Clear_BackBuffer_View(const Float4* pClearColor);

    /* 깊이버퍼 + 스텐실버퍼를 지운다. */
    HRESULT Clear_DepthStencil_View();

    /* 후면 버퍼를 전면버퍼로 교체한다.(백버퍼를 화면에 직접 보여준다.) */
    HRESULT Present();

    /* 창 크기 변경 (Windows WM_SIZE / Android Surface 변경) */
    void    Resize(uint32 iWinSizeX, uint32 iWinSizeY);

    ComPtr<RTV>                 GetRTV();
    ComPtr<DepthStencilView>    GetDSV();

    /* 원본 GetBuffer(ID3D11Texture2D) 대응 : 현재 백버퍼 텍스처 */
    Texture2D*                  Get_BackBuffer();

    ComPtr<Device>              Get_Device() { return m_pDevice; }
    ComPtr<DeviceContext>       Get_DeviceContext() { return m_pDeviceContext; }
    Diligent::ISwapChain*       Get_SwapChain() { return m_pSwapChain.Get(); }

    const VIEWPORT_DESC&        Get_OriginViewPort() const { return m_tOriginViewPort; }

    void                        Set_ScissorRectsOrigin();

    GRAPHIC_API                 Get_GraphicAPI() const { return m_eGraphicAPI; }
    uint64                      Get_FrameIndex() const { return m_iFrameIndex; }

public:
    HWND        Get_HWND() { return m_hWnd;}
    HANDLE      Get_Handle() { return m_hInst; }

    HRESULT     Set_GPU_OriginViewPort();

#pragma region DX11 State Machine (Engine 내부 전용)
public:
    static CGraphicDevice*      Get_Instance() { return s_pInstance; }

    /* OMSetRenderTargets / OMGetRenderTargets */
    void                        OM_SetRenderTargets(uint32 iNumRTV, ShaderResourceView* const* ppRTVs, DepthStencilView* pDSV);
    void                        OM_GetRenderTargets(uint32 iNumRTV, RenderTargetView** ppRTVs, DepthStencilView** ppDSV) const;
    void                        OM_SetBlendState(const BLEND_DESC& tDesc, const float* pBlendFactor = nullptr);
    void                        OM_SetDepthStencilState(const DEPTH_STENCIL_DESC& tDesc, uint32 iStencilRef = 0);

    /* RSSetViewports / RSSetScissorRects / RSSetState */
    void                        RS_SetViewports(uint32 iNumViewports, const VIEWPORT_DESC* pViewports);
    void                        RS_GetViewports(uint32* pNumViewports, VIEWPORT_DESC* pViewports) const;
    void                        RS_SetScissorRects(uint32 iNumRects, const RECT_DESC* pRects);
    void                        RS_SetState(const RASTERIZER_DESC& tDesc);

    /* IA */
    void                        IA_SetVertexBuffers(uint32 iStartSlot, uint32 iNumBuffers, Buffer* const* ppBuffers, const uint32* pStrides, const uint64* pOffsets);
    void                        IA_SetIndexBuffer(Buffer* pIndexBuffer, Diligent::VALUE_TYPE eIndexType, uint64 iOffset);
    void                        IA_SetPrimitiveTopology(PRIMITIVE_TOPOLOGY eTopology);

    /* Effect Pass Apply 결과 (어떤 셰이더의 어떤 패스로 그릴지) */
    void                        Set_CurrentPass(CShader* pShader, uint32 iPassIndex);

    /* Draw */
    HRESULT                     Draw(uint32 iVertexCount, uint32 iStartVertex);
    HRESULT                     DrawIndexed(uint32 iIndexCount, uint32 iStartIndex, int32 iBaseVertex);
    HRESULT                     DrawInstanced(uint32 iVertexCountPerInstance, uint32 iInstanceCount, uint32 iStartVertex, uint32 iStartInstance);
    HRESULT                     DrawIndexedInstanced(uint32 iIndexCountPerInstance, uint32 iInstanceCount, uint32 iStartIndex, int32 iBaseVertex, uint32 iStartInstance);

    /* 화면 일부만 지우기 (A-plan Clear_MRT_Rect) */
    HRESULT                     Clear_RenderTarget_Rect(ShaderResourceView* pRTV, const Color& vColor, const RECT_DESC& tRect);

    const PIPELINE_STATE&       Get_PipelineState() const { return m_tState; }
    /* 엔진 밖에서 직접 PSO 를 바꾼 경우 (PrimitiveBatch , ImGui) 캐시를 무효화한다 */
    void                        Invalidate_BoundState();

    /* 정해진 Format 이 Render Target 으로 지원되지 않으면 지원되는 Format 으로 바꿔준다 (Android Vulkan 대응) */
    TEXTURE_FORMAT              Get_SupportedRenderTargetFormat(TEXTURE_FORMAT eFormat) const;
    bool                        Is_TextureFormatSupported(TEXTURE_FORMAT eFormat) const;
private:
    HRESULT                     Flush_Draw();
    void                        Apply_ViewportAndScissor();
    void                        Update_RenderTargetSize();
#pragma endregion

private:
    HRESULT Initialize_Vulkan(const GraphicDevice_Desc& tDesc);
#if defined(_WIN32)
    HRESULT Initialize_DirectX11(const GraphicDevice_Desc& tDesc);
#endif

private:
    static CGraphicDevice*                  s_pInstance;

    ComPtr<Device>                          m_pDevice;
    ComPtr<DeviceContext>                   m_pDeviceContext;
    ComPtr<Diligent::ISwapChain>            m_pSwapChain;

private:
    uint32                    m_iWinSizeX; // 화면 전체 사이즈 X
    uint32                    m_iWinSizeY; // 화면 전체 사이즈 Y

    VIEWPORT_DESC             m_tOriginViewPort;

    GRAPHIC_API               m_eGraphicAPI;
    uint64                    m_iFrameIndex;

private:
    HWND                      m_hWnd;
    HANDLE                    m_hInst;

private:
    /* ---- 현재 바인딩 상태 ---- */
    PIPELINE_STATE            m_tState;
    ShaderResourceView*       m_pCurRTVs[8];
    DepthStencilView*         m_pCurDSV;
    uint32                    m_iRTWidth;
    uint32                    m_iRTHeight;

    VIEWPORT_DESC             m_tCurViewPort;
    bool                      m_isScissorSet;
    RECT_DESC                 m_tCurScissor;

    Buffer*                   m_pCurVertexBuffers[2];
    uint64                    m_iCurVertexOffsets[2];
    Buffer*                   m_pCurIndexBuffer;
    Diligent::VALUE_TYPE      m_eCurIndexType;

    CShader*                  m_pCurShader;
    uint32                    m_iCurPass;

    Diligent::IPipelineState* m_pBoundPSO;

public:
    static CGraphicDevice* Create( CONST GraphicDevice_Desc& tDesc, ComPtr<Device>* pDevice, ComPtr<DeviceContext>* pDeviceContextOut);
    virtual void Free() override;
};

NS_END
