#pragma once

#include "CBase.h"
#include "Engine_Enum.h"

class CResource;

class ENGINE_DLL CResourceManager final : public CBase
{
private:
    explicit CResourceManager(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);

    CResource* Find_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType) const;
    HRESULT Ready_Default_Shader();
    HRESULT Ready_Default_Texture();
    HRESULT Ready_Default_Material();
    HRESULT Ready_Default_Model();

public:
    ~CResourceManager() override = default;
    HRESULT Ready_DefaultResource();
    CResource* Get_Resource(const std::wstring& strKey,
                            RESOURCE_TYPE eResourceType,
                            bool isCopy = false,
                            void* pArg = nullptr) const;
    HRESULT Add_Resource(const std::wstring& strKey, CResource* pResource);
    HRESULT Remove_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType);
    void Clear_Resource(RESOURCE_TYPE eResourceType);

    static CResourceManager* Create(Diligent::IRenderDevice* pDevice,
                                    Diligent::IDeviceContext* pDeviceContext);
    void Free() override;

private:
    std::array<std::unordered_map<std::wstring, CResource*>, static_cast<size_t>(RESOURCE_TYPE::END)> m_arrayResourceMap;
    Diligent::IRenderDevice* m_pDevice = nullptr;
    Diligent::IDeviceContext* m_pDeviceContext = nullptr;
};
