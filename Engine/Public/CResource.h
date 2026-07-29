#pragma once

#include "CBase.h"
#include "Engine_Enum.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CResource abstract : public CBase
{
protected:
    explicit CResource(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);
    explicit CResource(const CResource& Prototype);
    ~CResource() override = default;

    virtual HRESULT Proto_Initialize();
    virtual HRESULT Copy_Initialize(void* pArg);

public:
    MAKE_TYPE Get_MakeType() const { return m_eMakeType; }
    RESOURCE_TYPE Get_ResourceType() const { return m_eResourceType; }
    const std::wstring& Get_Path() const { return m_strPath; }

    virtual CResource* Copy_Resource(void* pArg = nullptr) = 0;
    void Free() override;

protected:
    MAKE_TYPE m_eMakeType = MAKE_TYPE::PROTOTYPE;
    RESOURCE_TYPE m_eResourceType = RESOURCE_TYPE::END;
    std::wstring m_strPath;

    // Diligent 객체는 자체 참조 카운트를 사용하므로 엔진 DLL 경계에는 Raw Pointer만 보관한다.
    // 실제 수명은 CGraphicDevice가 엔진 종료 때까지 보장한다.
    Diligent::IRenderDevice* m_pDevice = nullptr;
    Diligent::IDeviceContext* m_pDeviceContext = nullptr;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
