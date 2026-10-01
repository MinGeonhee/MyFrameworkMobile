#include "CComponent.h"
#include "MyImGuiHeader.h"
#include "EnumUtils.h"
#include "CImGuiFuntion.h"
#include "CGameObject.h"
#include "CCameraObjectBase.h"

CComponent::CComponent(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pOwner(nullptr) , m_eMakeType(MAKE_TYPE::PROTOTYPE) , m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_isActive(true)
{
}

CComponent::CComponent(const CComponent& Prototype)
	: m_pOwner(Prototype.m_pOwner) , m_eMakeType(MAKE_TYPE::CLONE)
	, m_pDevice(Prototype.m_pDevice), m_pDeviceContext(Prototype.m_pDeviceContext) , m_isActive(Prototype.m_isActive)
{
}

CComponent::~CComponent()
{
}

void CComponent::Set_Owner(CBase* pOwner)
{
	NULLPTR_RETURN_NULL(pOwner);

	m_pOwner = pOwner;
}


HRESULT CComponent::Proto_Initialize()
{
	return S_OK;
}

HRESULT CComponent::Clone_Initialize(void* pArg)
{
	//NULLPTR_RETURN_EFAIL(pArg);


	return S_OK;
}


void    CComponent::Pre_Update(float DT)
{
	return;
}
void    CComponent::Update(float DT)
{
	return;
}
void    CComponent::Late_Update(float DT)
{
	return;
}

HRESULT CComponent::Render(FD CCameraObjectBase* pRenderCam)
{

	return S_OK;
}


#ifdef _DEBUG
void CComponent::ImGuiUpdate()
{
	return;
}
#endif

void CComponent::Free()
{
	__super::Free();

	m_pOwner = nullptr;


	return;
}