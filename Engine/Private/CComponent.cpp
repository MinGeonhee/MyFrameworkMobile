#include "pch.h"
#include "CComponent.h"

CComponent::CComponent(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
{
}

CComponent::CComponent(const CComponent& Prototype)
    : CBase{Prototype}
    , m_pDevice{Prototype.m_pDevice}
    , m_pDeviceContext{Prototype.m_pDeviceContext}
    , m_isActive{Prototype.m_isActive}
{
}

void CComponent::Set_Owner(CBase* pOwner)
{
    m_pOwner = pOwner;
}

HRESULT CComponent::Proto_Initialize()
{
    return S_OK;
}

HRESULT CComponent::Clone_Initialize(void*)
{
    return S_OK;
}

void CComponent::Pre_Update(float)
{
}

void CComponent::Update(float)
{
}

void CComponent::Late_Update(float)
{
}

HRESULT CComponent::Render()
{
    return S_OK;
}

void CComponent::Free()
{
    m_pOwner = nullptr;
    m_pDevice = nullptr;
    m_pDeviceContext = nullptr;
    CBase::Free();
}
