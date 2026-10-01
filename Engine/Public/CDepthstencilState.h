#pragma once
#include "CRenderState.h"

NS_BEGIN(Engine)

class ENGINE_DLL CDepthStencilState final : public CRenderState
{
private:
	explicit CDepthStencilState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CDepthStencilState() {}
private:
	HRESULT							Initialize(const DEPTH_STENCIL_DESC& tDesc);
public:
	virtual HRESULT							Set_GPU_RenderState()override;
	const DEPTH_STENCIL_DESC&				Get_Desc() { return m_tDepthstencilState_Desc; }
	HRESULT									Set_GPU_DepthstencilState()
	{
		return Set_GPU_RenderState();
	}
private:
	DEPTH_STENCIL_DESC						m_tDepthstencilState_Desc;
public:
	static CDepthStencilState* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, const DEPTH_STENCIL_DESC& tDesc);
	virtual void Free() override;
};

NS_END
