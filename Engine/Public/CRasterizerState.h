#pragma once
#include "CRenderState.h"

NS_BEGIN(Engine)

class ENGINE_DLL CRasterizerState : public CRenderState
{
private:
	explicit						CRasterizerState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual							~CRasterizerState() {}
private:
	HRESULT							Initialize(const RASTERIZER_DESC& tDesc);
public:
	virtual HRESULT					Set_GPU_RenderState()override;
	const RASTERIZER_DESC&			Get_Desc() { return m_tRasterizerState_Desc; }
	HRESULT							Set_GPU_RasterizerState()
	{
		return Set_GPU_RenderState();
	}
protected:
	RASTERIZER_DESC					m_tRasterizerState_Desc;
public:
	static CRasterizerState* Create(const RASTERIZER_DESC& tDesc, ComPtr<Device>pDevce, ComPtr<DeviceContext>pContext);
	virtual void Free() override;
};

NS_END
