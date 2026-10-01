#include "CBlendState.h"
#include "CGraphicDevice.h"

CBlendState::CBlendState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CRenderState(pDevice,pContext), m_pBlendState_Desc{}, m_fBlendFactor(0.f), m_iSampleMask(0xFFFFFFFF)
{
	m_eStateType = STATE_TYPE::BLEND;
}

HRESULT CBlendState::Initialize(const BLEND_DESC& tDesc, float fBlendFactor, uint32 iSampleMask)
{
	m_pBlendState_Desc = tDesc;
	m_fBlendFactor = fBlendFactor;
	m_iSampleMask = iSampleMask;

	return S_OK;
}

HRESULT CBlendState::Set_GPU_RenderState()
{
	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	NULLPTR_RETURN_EFAIL(pGraphicDevice);

	const float fBlendFactor[4] = { m_fBlendFactor, m_fBlendFactor, m_fBlendFactor, m_fBlendFactor };
	pGraphicDevice->OM_SetBlendState(m_pBlendState_Desc, fBlendFactor);

	return S_OK;
}

CBlendState* CBlendState::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, const BLEND_DESC& tDesc, float fBlendFactor, uint32 iSampleMask)
{
	CBlendState* pBlendState = new CBlendState(pDevice,pContext);

	if (FAILED(pBlendState->Initialize(tDesc, fBlendFactor, iSampleMask)))
	{
		Safe_Release(pBlendState);
		MSG_BOX("BlendState is failed to Create");
		return nullptr;
	}

	return pBlendState;
}

void CBlendState::Free()
{
	__super::Free();
	/* CBlendState::Free() */
}
