#pragma once
#include "CBase.h"


NS_BEGIN(Engine)


class ENGINE_DLL CRenderState abstract : public CBase
{
protected:
	explicit CRenderState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CRenderState() {}
public:
	STATE_TYPE						Get_StateType() { return m_eStateType; }
	virtual HRESULT					Set_GPU_RenderState() PURE;
protected:
	STATE_TYPE						m_eStateType;
protected:
	ComPtr<Device>					m_pDevice;
	ComPtr<DeviceContext>			m_pDeviceContext;
};

NS_END