#include "CBounding.h"



CBounding::CBounding(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext)
{
}

CBounding::~CBounding()
{
}


HRESULT CBounding::Initialize()
{
	return S_OK;
}


void CBounding::Free()
{
	__super::Free();

	/* CBoundging::Free() */
}
