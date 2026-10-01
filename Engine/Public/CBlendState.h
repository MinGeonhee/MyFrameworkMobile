#pragma once
#include "CRenderState.h"

NS_BEGIN(Engine)

/* 원본 : ID3D11BlendState 를 만들어 OMSetBlendState 했다.                                    */
/* 현재 : Desc 만 들고 있다가 GraphicDevice 의 "현재 Blend 상태" 로 넣어준다 (Draw 때 PSO 로 묶인다) */
class ENGINE_DLL CBlendState :    public CRenderState
{
private:
	explicit CBlendState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CBlendState() {}
private:
	HRESULT							Initialize(const BLEND_DESC& tDesc, float fBlendFactor, uint32 iSampleMask);

public:
	virtual HRESULT					Set_GPU_RenderState()override;
	const BLEND_DESC&				Get_Desc()		 { return m_pBlendState_Desc; }

	HRESULT							Set_GPU_BlendState()
	{
		return Set_GPU_RenderState();
	}
private:
	float									m_fBlendFactor;
	uint32									m_iSampleMask;
private:
	BLEND_DESC								m_pBlendState_Desc;
public:
	static CBlendState* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContex,const BLEND_DESC& tDesc, float fBlendFactor=0 , uint32 iSampleMask = 0xFFFFFFFF);
	virtual void Free() override;
};

NS_END
