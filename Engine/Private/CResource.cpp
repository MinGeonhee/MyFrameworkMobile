#include "pch.h"
#include "CResource.h"

CResource::CResource(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
{
}

CResource::CResource(const CResource& Prototype)
    : CBase{Prototype}
    , m_eMakeType{MAKE_TYPE::CLONE}
    , m_eResourceType{Prototype.m_eResourceType}
    , m_strPath{Prototype.m_strPath}
    , m_pDevice{Prototype.m_pDevice}
    , m_pDeviceContext{Prototype.m_pDeviceContext}
{
}

HRESULT CResource::Proto_Initialize()
{
    return S_OK;
}

HRESULT CResource::Copy_Initialize(void*)
{
    return S_OK;
}

void CResource::Free()
{
    m_pDeviceContext = nullptr;
    m_pDevice = nullptr;
    CBase::Free();
}
