#include "pch.h"
#include "CTexture.h"
#include "CShader.h"
#include "Engine_Android.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

// DiligentTools???대??곸쑝濡??ъ슜?섎뒗 stb_image瑜?吏곸젒 ?ъ슜?쒕떎.
// Windows ?꾩슜 WIC/DDSTextureLoader瑜??쒓굅?댁빞 Android?먯꽌??媛숈? CTexture 肄붾뱶媛 ?숈옉?쒕떎.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
namespace
{
#if defined(__ANDROID__)
std::string ToUTF8AssetPath(const std::filesystem::path& FilePath)
{
    std::string Path = FilePath.u8string();
    std::replace(Path.begin(), Path.end(), '\\', '/');

    const std::string ResourceToken = "Resource/";
    const size_t ResourcePos = Path.find(ResourceToken);
    if (ResourcePos != std::string::npos)
        return Path.substr(ResourcePos);
    return Path;
}

stbi_uc* Load_AndroidAsset(const std::filesystem::path& FilePath, int* pWidth, int* pHeight, int* pChannels)
{
    AAssetManager* pAssetManager = Engine::Get_Android_AssetManager();
    if (pAssetManager == nullptr)
        return nullptr;

    const std::string AssetPath = ToUTF8AssetPath(FilePath);
    AAsset* pAsset = AAssetManager_open(pAssetManager, AssetPath.c_str(), AASSET_MODE_BUFFER);
    if (pAsset == nullptr)
        return nullptr;

    const void* pBuffer = AAsset_getBuffer(pAsset);
    const off_t iLength = AAsset_getLength(pAsset);
    stbi_uc* pPixels = stbi_load_from_memory(
        static_cast<const stbi_uc*>(pBuffer),
        static_cast<int>(iLength),
        pWidth,
        pHeight,
        pChannels,
        STBI_rgb_alpha);
    AAsset_close(pAsset);
    return pPixels;
}
#endif
}

CTexture::CTexture(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext)
    : CResource{pDevice, pDeviceContext}
{
    m_eResourceType = RESOURCE_TYPE::TEXTURE;
}

CTexture::CTexture(const CTexture& Prototype)
    : CResource{Prototype}
    , m_strFileName{Prototype.m_strFileName}
    , m_vecTexture{Prototype.m_vecTexture}
{
}

HRESULT CTexture::Initialize(const std::wstring& strPath, uint32 iTextureCount)
{
    if (m_pDevice == nullptr || strPath.empty() || iTextureCount == 0)
        return E_INVALIDARG;

    for (uint32 i = 0; i < iTextureCount; ++i)
    {
        wchar_t szPath[MAX_PATH]{};
        if (iTextureCount > 1)
        {
#if defined(_WIN32)
            swprintf_s(szPath, strPath.c_str(), i);
#else
            std::swprintf(szPath, MAX_PATH, strPath.c_str(), i);
#endif
        }
        else
        {
#if defined(_WIN32)
            wcscpy_s(szPath, strPath.c_str());
#else
            std::wcsncpy(szPath, strPath.c_str(), MAX_PATH - 1);
            szPath[MAX_PATH - 1] = L'\0';
#endif
        }

        const std::filesystem::path FilePath{szPath};
        const std::string UTF8Path = FilePath.u8string();
        int iWidth = 0;
        int iHeight = 0;
        int iChannels = 0;
        stbi_uc* pPixels = stbi_load(UTF8Path.c_str(), &iWidth, &iHeight, &iChannels, STBI_rgb_alpha);
#if defined(__ANDROID__)
        if (pPixels == nullptr)
            pPixels = Load_AndroidAsset(FilePath, &iWidth, &iHeight, &iChannels);
#endif
        if (pPixels == nullptr || iWidth <= 0 || iHeight <= 0)
        {
            stbi_image_free(pPixels);
            return E_FAIL;
        }

        const HRESULT hr = Initialize_RGBA8(
            pPixels,
            static_cast<uint32>(iWidth),
            static_cast<uint32>(iHeight),
            true); // ?쇰컲 而щ윭 ?띿뒪泥섎뒗 sRGB濡??댁꽍?쒕떎.
        stbi_image_free(pPixels);
        if (FAILED(hr))
            return hr;
    }

    m_strPath = strPath;
    m_strFileName = std::filesystem::path{strPath}.filename().wstring();
    return S_OK;
}

HRESULT CTexture::Initialize(Diligent::ITexture* pTexture)
{
    if (pTexture == nullptr)
        return E_INVALIDARG;
    m_vecTexture.emplace_back(pTexture);
    return S_OK;
}

HRESULT CTexture::Initialize_RGBA8(const void* pPixels, uint32 iWidth, uint32 iHeight, bool isSRGB)
{
    if (m_pDevice == nullptr || pPixels == nullptr || iWidth == 0 || iHeight == 0)
        return E_INVALIDARG;

    Diligent::TextureDesc Desc{};
    Desc.Name = "Engine Texture";
    Desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    Desc.Width = iWidth;
    Desc.Height = iHeight;
    Desc.MipLevels = 1;
    Desc.Format = isSRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM;
    Desc.Usage = Diligent::USAGE_IMMUTABLE;
    Desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

    Diligent::TextureSubResData SubResource{};
    SubResource.pData = pPixels;
    SubResource.Stride = static_cast<Diligent::Uint64>(iWidth) * 4u;
    Diligent::TextureData Data{&SubResource, 1};

    Diligent::RefCntAutoPtr<Diligent::ITexture> pTexture;
    m_pDevice->CreateTexture(Desc, &Data, &pTexture);
    if (pTexture == nullptr)
        return E_FAIL;
    m_vecTexture.push_back(std::move(pTexture));
    return S_OK;
}

HRESULT CTexture::Copy_Initialize(void* pArg)
{
    return CResource::Copy_Initialize(pArg);
}

Diligent::ITextureView* CTexture::GetShaderResourceView(uint32 iIndex) const
{
    if (iIndex >= m_vecTexture.size() || m_vecTexture[iIndex] == nullptr)
        return nullptr;
    return m_vecTexture[iIndex]->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
}

Diligent::ITexture* CTexture::Get_Texture(uint32 iIndex) const
{
    return iIndex < m_vecTexture.size() ? m_vecTexture[iIndex].RawPtr() : nullptr;
}

HRESULT CTexture::Set_GPU_Texture(CShader* pShader, const std::string& strVariableName)
{
    return pShader != nullptr ? pShader->Set_GPU_Texture(strVariableName, GetShaderResourceView()) : E_INVALIDARG;
}

HRESULT CTexture::Set_GPU_Textures(CShader* pShader, uint32 iCountTex, uint32 iOffset)
{
    if (pShader == nullptr || iCountTex == 0 || iOffset + iCountTex > m_vecTexture.size())
        return E_INVALIDARG;

    std::vector<Diligent::ITextureView*> Views;
    Views.reserve(iCountTex);
    for (uint32 i = 0; i < iCountTex; ++i)
        Views.push_back(GetShaderResourceView(iOffset + i));
    return pShader->Set_GPU_Textures(iCountTex, Views.data(), 0);
}

CTexture* CTexture::Create(const std::wstring& strPath,
                           uint32 iTextureCount,
                           Diligent::IRenderDevice* pDevice,
                           Diligent::IDeviceContext* pDeviceContext)
{
    CTexture* pInstance = new CTexture{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(strPath, iTextureCount)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CTexture* CTexture::Create(Diligent::ITexture* pTexture,
                           Diligent::IRenderDevice* pDevice,
                           Diligent::IDeviceContext* pDeviceContext)
{
    CTexture* pInstance = new CTexture{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(pTexture)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CTexture* CTexture::Create_RGBA8(const void* pPixels,
                                 uint32 iWidth,
                                 uint32 iHeight,
                                 bool isSRGB,
                                 Diligent::IRenderDevice* pDevice,
                                 Diligent::IDeviceContext* pDeviceContext)
{
    CTexture* pInstance = new CTexture{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize_RGBA8(pPixels, iWidth, iHeight, isSRGB)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CResource* CTexture::Copy_Resource(void* pArg)
{
    CTexture* pCopy = new CTexture{*this};
    if (FAILED(pCopy->Copy_Initialize(pArg)))
    {
        pCopy->Release();
        return nullptr;
    }
    return pCopy;
}

void CTexture::Free()
{
    m_vecTexture.clear();
    CResource::Free();
}


