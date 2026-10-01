#include "CFontManager.h"
#include "CFont.h"

CFontManager::CFontManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext) , m_pSpriteBatch(nullptr)
{

}


HRESULT	CFontManager::Initialize()
{
    m_pSpriteBatch = new SpriteBatch(m_pDeviceContext.Get());

    return S_OK;
}

HRESULT CFontManager::Add_Font(float fOriginFontSize,const wstring& wstrAddKey, const wstring& wstrFontPath)
{
    CFont* pFind = Get_Font(wstrAddKey);
    if (pFind != nullptr) return E_FAIL;

    pFind = CFont::Create(fOriginFontSize,wstrFontPath,m_pDevice,m_pDeviceContext);

    m_umapFont.emplace(wstrAddKey, pFind);

    return S_OK;
}

CFont* CFontManager::Get_Font(const wstring& wstrFindKey)
{
    auto iter = m_umapFont.find(wstrFindKey);
    if (iter == m_umapFont.end()) return nullptr;
    return iter->second;
}

HRESULT CFontManager::Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const Float2& vSize, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{

    CFont* pFindFont = Get_Font(wstringFindKey);
    NULLPTR_RETURN_EFAIL(pFindFont);
    m_pSpriteBatch->Begin();
    pFindFont->Draw(m_pSpriteBatch, pText, vPosition, vSize ,vColor,eType);
    m_pSpriteBatch->End();

    return S_OK;
}

HRESULT CFontManager::Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{
    CFont* pFindFont = Get_Font(wstringFindKey);
    NULLPTR_RETURN_EFAIL(pFindFont);
    m_pSpriteBatch->Begin();
    pFindFont->Draw(m_pSpriteBatch,pText,vPosition,vColor, eType);
    m_pSpriteBatch->End();

    return S_OK;
}


CFontManager* CFontManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CFontManager* pFontManager = new CFontManager(pDevice,pContext);

    if (FAILED(pFontManager->Initialize()))
    {
        Safe_Release(pFontManager);
        MSG_BOX("Font Manger is failed to Create");
        return nullptr;
    }

    return pFontManager;
}

void CFontManager::Free()
{
    __super::Free();

    for (auto& Font : m_umapFont)
        Safe_Release(Font.second);

    Safe_Delete(m_pSpriteBatch);
}
