#pragma once

#include "CResource.h"
#include "Engine_Enum.h"

class CShader;
class CTexture;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CMaterial final : public CResource
{
private:
    explicit CMaterial(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);
    explicit CMaterial(const CMaterial& Prototype);
    ~CMaterial() override = default;
    HRESULT Initialize(const std::wstring& strName);
    HRESULT Copy_Initialize(void* pArg) override;

public:
    const std::wstring& Get_Name() const { return m_strName; }
    HRESULT Set_Texture(MATERIAL_TYPE eType, CTexture* pTexture);
    CTexture* Get_Texture(MATERIAL_TYPE eType) const;
    HRESULT Set_GPU_Material(CShader* pShader);

    static CMaterial* Create(const std::wstring& strName,
                             Diligent::IRenderDevice* pDevice,
                             Diligent::IDeviceContext* pDeviceContext);
    CResource* Copy_Resource(void* pArg = nullptr) override;
    void Free() override;

private:
    std::wstring m_strName;
    std::array<CTexture*, static_cast<size_t>(MATERIAL_TYPE::END)> m_arrayTexture{};
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
