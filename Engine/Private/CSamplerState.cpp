#include "CSamplerState.h"

CSamplerState::CSamplerState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CRenderState(pDevice, pContext), m_tSamplerState_Desc{}
{
	m_eStateType = STATE_TYPE::SAMPLER;
}

HRESULT CSamplerState::Initialize(CONST SAMPLER_DESC& tDesc)
{
	m_tSamplerState_Desc = tDesc;

	return S_OK;
}

HRESULT CSamplerState::Set_GPU_RenderState()
{
	return S_OK;
}

HRESULT CSamplerState::Set_GPU_SamplerState(SHADER_TYPE eShaderType, uint32 iSlot)
{
	/* 셰이더 파일의 sampler_state 가 Immutable Sampler 로 고정된다 (Diligent PSO) */
	return S_OK;
}

CSamplerState* CSamplerState::Create(CONST SAMPLER_DESC& tDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CSamplerState* pSamplerState = new CSamplerState(pDevice, pContext);

	if (FAILED(pSamplerState->Initialize(tDesc)))
	{
		Safe_Release(pSamplerState);
		MSG_BOX("pSamplerState is failed to Create");
		return nullptr;
	}

	return pSamplerState;
}

void CSamplerState::Free()
{
	__super::Free();
	/*  CSamplerState::Free()*/
}
