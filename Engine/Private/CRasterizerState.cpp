#include "CRasterizerState.h"
#include "CGraphicDevice.h"

CRasterizerState::CRasterizerState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CRenderState(pDevice, pContext), m_tRasterizerState_Desc{}
{
    m_eStateType = STATE_TYPE::RASTERIZER;
}

HRESULT CRasterizerState::Initialize(const RASTERIZER_DESC& tDesc)
{
    m_tRasterizerState_Desc = tDesc;

    return S_OK;
}

HRESULT CRasterizerState::Set_GPU_RenderState()
{
    CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
    NULLPTR_RETURN_EFAIL(pGraphicDevice);

    pGraphicDevice->RS_SetState(m_tRasterizerState_Desc);
    return S_OK;
}

CRasterizerState* CRasterizerState::Create(const RASTERIZER_DESC& tDesc, ComPtr<Device> pDevce, ComPtr<DeviceContext> pContext)
{
    CRasterizerState* pRasterizerState = new CRasterizerState(pDevce, pContext);

    if (FAILED(pRasterizerState->Initialize(tDesc)))
    {
        Safe_Release(pRasterizerState);
        MSG_BOX("RasterizerState is failed to Create");
    }

    return pRasterizerState;
}

void CRasterizerState::Free()
{
    __super::Free();
}
