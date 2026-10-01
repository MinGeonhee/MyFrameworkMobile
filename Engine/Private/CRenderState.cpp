#include "CRenderState.h"

CRenderState::CRenderState(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_eStateType(STATE_TYPE::END)
{

}
