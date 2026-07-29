#pragma once

#include "CResource.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class CShader;

class ENGINE_DLL CTexture final : public CResource
{
private:
    explicit CTexture(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);
    explicit CTexture(const CTexture& Prototype);
    ~CTexture() override = default;

    HRESULT Initialize(const std::wstring& strPath, uint32 iTextureCount);
    HRESULT Initialize(Diligent::ITexture* pTexture);
    HRESULT Initialize_RGBA8(const void* pPixels, uint32 iWidth, uint32 iHeight, bool isSRGB);
    HRESULT Copy_Initialize(void* pArg) override;

public:
    Diligent::ITextureView* GetShaderResourceView(uint32 iIndex = 0) const;
    Diligent::ITexture* Get_Texture(uint32 iIndex = 0) const;
    uint32 Get_TextureCount() const { return static_cast<uint32>(m_vecTexture.size()); }

    HRESULT Set_GPU_Texture(CShader* pShader, const std::string& strVariableName = "g_Texture");
    HRESULT Set_GPU_Textures(CShader* pShader, uint32 iCountTex, uint32 iOffset = 0);

    static CTexture* Create(const std::wstring& strPath,
                            uint32 iTextureCount,
                            Diligent::IRenderDevice* pDevice,
                            Diligent::IDeviceContext* pDeviceContext);
    static CTexture* Create(Diligent::ITexture* pTexture,
                            Diligent::IRenderDevice* pDevice,
                            Diligent::IDeviceContext* pDeviceContext);
    static CTexture* Create_RGBA8(const void* pPixels,
                                  uint32 iWidth,
                                  uint32 iHeight,
                                  bool isSRGB,
                                  Diligent::IRenderDevice* pDevice,
                                  Diligent::IDeviceContext* pDeviceContext);

    CResource* Copy_Resource(void* pArg = nullptr) override;
    void Free() override;

private:
    std::wstring m_strFileName;
    std::vector<Diligent::RefCntAutoPtr<Diligent::ITexture>> m_vecTexture;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
