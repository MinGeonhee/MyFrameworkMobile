#include "CRenderStateManager.h"
#include "CRasterizerState.h"
#include "CBlendState.h"
#include "CSamplerState.h"
#include "CDepthstencilState.h"


CRenderStateManager::CRenderStateManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext)
{
}

HRESULT CRenderStateManager::Initialize()
{

    return S_OK;
}

HRESULT CRenderStateManager::Set_GPU_RenderState(uint32 iIndex, STATE_TYPE eType)
{
    CRenderState* pRenderState = Get_RenderState(iIndex, eType);
    NULLPTR_RETURN_EFAIL(pRenderState);
    return pRenderState->Set_GPU_RenderState();
}

HRESULT CRenderStateManager::Add_RenderState(uint32 iIndex, CRenderState* pAddRenderState)
{
    NULLPTR_RETURN_EFAIL(pAddRenderState);

    STATE_TYPE eAddType = pAddRenderState->Get_StateType();

    CRenderState* pFind = Get_RenderState(iIndex , eAddType );

    if (pFind != nullptr) return E_FAIL;

    m_umapRenderState[Cast_Uint(eAddType)].emplace(iIndex , pAddRenderState);

    return S_OK;
}

CRenderState* CRenderStateManager::Get_RenderState(uint32 iIndex , STATE_TYPE eType)
{
    const auto& iter = m_umapRenderState[Cast_Uint(eType)].find(iIndex);

    if (iter == m_umapRenderState[Cast_Uint(eType)].end())
        return nullptr;


    return iter->second;
}

CRenderStateManager* CRenderStateManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CRenderStateManager* pStateManager = new CRenderStateManager(pDevice, pContext);

    if ( FAILED(pStateManager->Initialize()))
    {
        Safe_Release(pStateManager);
        MSG_BOX("State Manager is failed to Create");
        return nullptr;
    }


    return pStateManager;
}

void CRenderStateManager::Free()
{
    for (auto& RenderStateMap : m_umapRenderState)
        for (auto& Pair : RenderStateMap)
            Safe_Release(Pair.second);
}
