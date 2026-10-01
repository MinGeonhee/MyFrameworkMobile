#pragma once
#include "CRenderState.h"

NS_BEGIN(Engine)

/* Vulkan(Diligent) 에서는 셰이더 파일의 sampler_state 가 PSO 의 Immutable Sampler 로 들어간다.  */
/* 이 클래스는 원본처럼 Desc 를 보관하며 , 런타임 샘플러 교체(PSSetSamplers)는 지원하지 않는다.     */
class ENGINE_DLL CSamplerState : public CRenderState
{
private:
	explicit CSamplerState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CSamplerState() {}
private:
	HRESULT							Initialize(CONST SAMPLER_DESC& tDesc);
public:
	virtual HRESULT					Set_GPU_RenderState()override;
	const SAMPLER_DESC&				Get_Desc()			{ return m_tSamplerState_Desc; }
	HRESULT							Set_GPU_SamplerState(SHADER_TYPE eShaderType , uint32 iSlot);
private:
	SAMPLER_DESC					m_tSamplerState_Desc;
public:
	static CSamplerState* Create(CONST SAMPLER_DESC& tDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END
