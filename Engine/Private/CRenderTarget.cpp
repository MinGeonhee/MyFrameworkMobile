#include "CRenderTarget.h"
#include "CGameManager.h"
#include "CVIBuffer_Rect.h"
#include "CShader.h"
#include "CGraphicDevice.h"


CRenderTarget::CRenderTarget(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_pRTV(nullptr), m_pSRV(nullptr), m_pTexture2D(nullptr), m_vClearColor{}, m_iWidth(), m_iHeight(), m_eFormat()
#ifdef _DEBUG
    , m_matWorld{}
#endif
{
#ifdef _DEBUG
    XMStoreFloat4x4(&m_matWorld, XMMatrixIdentity());
#endif
}

HRESULT CRenderTarget::Initialize(uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat)
{
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_eFormat = eFormat;
    m_vClearColor = vColor;

    if(FAILED(Ready_Texture2D()))
        return E_FAIL;

    if(FAILED(Ready_ShaderResourceView()))
        return E_FAIL;
     

    if (FAILED(Ready_RenderTargetView()))
        return E_FAIL;
       
    return S_OK;
}

HRESULT CRenderTarget::Ready_Texture2D()
{
    /* Vulkan 에서 지원되지 않는 Render Target 형식이면 비슷한 형식으로 바꾼다 (예 : RGBA16_UNORM → RGBA16_FLOAT) */
    if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
        m_eFormat = pGraphicDevice->Get_SupportedRenderTargetFormat(m_eFormat);

    Diligent::TextureDesc tDesc{};
    tDesc.Name = "CRenderTarget";
    tDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    tDesc.Width = m_iWidth;
    tDesc.Height = m_iHeight;
    tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    tDesc.Usage = Diligent::USAGE_DEFAULT;
    tDesc.Format = m_eFormat;
    tDesc.MipLevels = 1;
    tDesc.ClearValue.Format = m_eFormat;
    tDesc.ClearValue.Color[0] = m_vClearColor.x;
    tDesc.ClearValue.Color[1] = m_vClearColor.y;
    tDesc.ClearValue.Color[2] = m_vClearColor.z;
    tDesc.ClearValue.Color[3] = m_vClearColor.w;

    m_pDevice->CreateTexture(tDesc, nullptr, &m_pTexture2D);
    if (m_pTexture2D == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CRenderTarget::Ready_ShaderResourceView()
{
    m_pSRV = m_pTexture2D->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    NULLPTR_RETURN_EFAIL(m_pSRV);
    m_pSRV->AddRef();
    return S_OK;
}

HRESULT CRenderTarget::Ready_RenderTargetView()
{
    m_pRTV = m_pTexture2D->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    NULLPTR_RETURN_EFAIL(m_pRTV);
    m_pRTV->AddRef();
    return S_OK;
}

HRESULT CRenderTarget::Clear_RenderTargetView()
{
    m_pDeviceContext->ClearRenderTarget(m_pRTV, reinterpret_cast<float*>(&m_vClearColor), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return S_OK;
}

HRESULT CRenderTarget::Bind_ShaderResource(const string& strBindTexName, CShader* pShader)
{
    return pShader->Set_GPU_Texture(strBindTexName, m_pSRV);
}




CRenderTarget* CRenderTarget::Create(uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CRenderTarget* pRenderTarget = new CRenderTarget(pDevice,pContext);

    if (FAILED(pRenderTarget->Initialize(iWidth, iHeight,vColor, eFormat)))
    {
        Safe_Release(pRenderTarget);
        MSG_BOX("Render Target View is Failed To Create");
        return nullptr;
    }

    return pRenderTarget;
}

void CRenderTarget::Free()
{
    __super::Free();

    Safe_Release(m_pSRV);
    Safe_Release(m_pRTV);
    Safe_Release(m_pTexture2D);
}




#ifdef _DEBUG
HRESULT CRenderTarget::Ready_Debug(float fX, float fY, float fSizeX, float fSizeY)
{
    VIEWPORT_DESC tOriginViewPort = CGameManager::GetInstance()->GraphicDevice_Get_OriginViewPort();

    XMStoreFloat4x4(&m_matWorld, XMMatrixScaling(fSizeX, fSizeY, 1.f) * XMMatrixTranslation(fX - tOriginViewPort.Width * 0.5f, -fY + tOriginViewPort.Height * 0.5f, 0.1f));

    return S_OK;
}
HRESULT CRenderTarget::Render(CShader* pShader, CVIBuffer_Rect* pBuffer)
{
    NULLPTR_RETURN_EFAIL(pShader); NULLPTR_RETURN_EFAIL(pBuffer);

    /* 내 월드행렬 던져주기 */
    pShader->Set_GPU_Transform(m_matWorld);
    /* 내가가진 RenderTargte에 그려진 ShaderResourceView를 텍스처 바인등으로 던져준다 */
    pShader->Set_GPU_Texture(m_pSRV);
    
    pShader->Apply_Pass("Debug");

    pBuffer->Draw();

    return S_OK;
}
#endif