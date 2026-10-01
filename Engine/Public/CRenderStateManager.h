#pragma once
#include "CBase.h"
#include "CSamplerState.h"
#include "CDepthstencilState.h"
#include "CBlendState.h"
#include "CRasterizerState.h"

namespace Engine
{
	class CRasterizerState;
	class CSamplerState;
	class CBlendState;
	class CDepthStencilState;
	class CRenderState;
}

NS_BEGIN(Engine)


class CRenderStateManager : public CBase
{
	explicit CRenderStateManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CRenderStateManager() = default;

private:
	HRESULT				Initialize();
public:
	HRESULT				Set_GPU_RenderState(uint32 iIndex, STATE_TYPE eType);
	HRESULT				Add_RenderState( uint32 iIndex , CRenderState* pAddRenderState );
	CRenderState*		Get_RenderState( uint32 iIndex , STATE_TYPE eType);
private:
	unordered_map<uint32 , CRenderState*> m_umapRenderState[Cast_Uint(STATE_TYPE::END)];
private:
	ComPtr<Device>			 m_pDevice;
private:
	ComPtr<DeviceContext>	 m_pDeviceContext;
public:
	static	CRenderStateManager*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void			Free() override;
};


NS_END
