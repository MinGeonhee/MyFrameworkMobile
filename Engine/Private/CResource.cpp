#include "CResource.h"
#include "CGameManager.h"


CResource::CResource(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_eMakeType(MAKE_TYPE::PROTOTYPE) , m_pGameManager(CGameManager::GetInstance())
	, m_szPath{}
{
	Safe_AddRef(m_pGameManager);
}

CResource::CResource(const CResource& Proto)
	: m_eMakeType(MAKE_TYPE::CLONE), m_eResourceType(Proto.m_eResourceType) , m_pGameManager(Proto.m_pGameManager)
{
	Safe_AddRef(m_pGameManager);
	strcpy_s(m_szPath,Proto.m_szPath);
}

CResource::~CResource()
{
}

HRESULT	CResource::Proto_Initialize()
{
	return S_OK;
}

HRESULT	CResource::Copy_Initialize(void* pArg)
{
	return S_OK;
}


void CResource::Free()
{
	__super::Free();
	/* CResource::Free() */
	Safe_Release(m_pGameManager);
}

