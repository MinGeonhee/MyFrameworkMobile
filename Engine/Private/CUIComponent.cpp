#include "CUIComponent.h"

CUIComponent::CUIComponent(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)\
	: CComponent(pDevice,pContext)
{
}

CUIComponent::CUIComponent(const CUIComponent& Proto)
	: CComponent(Proto)
{
}

CUIComponent::~CUIComponent()
{
}


HRESULT	CUIComponent::Proto_Initialize()
{
	return S_OK;
}

HRESULT CUIComponent::Clone_Initialize(void* pArg)
{
	return S_OK;
}


void CUIComponent::Pre_Update(float DT)
{
	__super::Pre_Update(DT);

	return;
}

void CUIComponent::Update(float DT)
{
	__super::Update(DT);

	return;
}

void CUIComponent::Late_Update(float DT)
{
	__super::Late_Update(DT);

	return;
}

HRESULT CUIComponent::Render(CCanvas* pCanvas)
{
	return S_OK;
}


void CUIComponent::Free()
{
	__super::Free();


	/* CUIComponent::Free() */
}
