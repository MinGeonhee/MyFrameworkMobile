#include "CDepthstencilState.h"
#include "CGraphicDevice.h"

CDepthStencilState::CDepthStencilState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	:  CRenderState(pDevice,pContext), m_tDepthstencilState_Desc{}
{
	m_eStateType = STATE_TYPE::DEAPTHSTENCIL;
}

HRESULT CDepthStencilState::Initialize(const DEPTH_STENCIL_DESC& tDesc)
{
	m_tDepthstencilState_Desc = tDesc;

	return S_OK;
}

HRESULT CDepthStencilState::Set_GPU_RenderState()
{
	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	NULLPTR_RETURN_EFAIL(pGraphicDevice);

	pGraphicDevice->OM_SetDepthStencilState(m_tDepthstencilState_Desc, 0);
	return S_OK;
}

CDepthStencilState* CDepthStencilState::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, const DEPTH_STENCIL_DESC& tDesc)
{
	CDepthStencilState* pDepthstencilState = new CDepthStencilState(pDevice, pContext);

	if (FAILED(pDepthstencilState->Initialize(tDesc)))
	{
		Safe_Release(pDepthstencilState);
		MSG_BOX("BlendState is failed to Create");
		return nullptr;
	}

	return pDepthstencilState;
}

void CDepthStencilState::Free()
{
	__super::Free();
	/* CDepthStencilState::Free() */
}
