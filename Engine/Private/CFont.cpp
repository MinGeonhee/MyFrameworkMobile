#include "CFont.h"


CFont::CFont(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_pSpriteFont(nullptr)
    , m_fOriginSize(0.f)
{
}

CFont::~CFont()
{
}

HRESULT CFont::Initialize(float fOriginFontSize,const wstring& wstrFontPath)
{
    /* Sprite Font 생성 */
    m_pSpriteFont = new SpriteFont(m_pDevice.Get(), wstrFontPath.c_str());
    NULLPTR_RETURN_EFAIL(m_pSpriteFont);


    m_fOriginSize = fOriginFontSize;

    return S_OK;
}

HRESULT CFont::Draw(SpriteBatch* pBatch, const TCHAR* pText, const Float2& vPosition, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{
    Float2 vPos = vPosition;

    XMFLOAT2 origin{0.f,0.f};

    if (eType == FONT_RENDER_TYPE::DEFUALT)
    {
        m_pSpriteFont->DrawString(pBatch, pText, vPos, vColor, 0.f, origin);
        return S_OK;
    }

    Float2 vTextSize{};
    XMStoreFloat2(&vTextSize, m_pSpriteFont->MeasureString(pText));


    switch (eType)
    {
    case Engine::FONT_RENDER_TYPE::DEFUALT: break;

    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER_X:
        vPos.x -= m_fOriginSize * 0.5f;
        origin = { vTextSize.x * 0.5f  , 0.f };
        break;
    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER_Y:
        origin = { 0.f , vTextSize.y * 0.5f };
        break;
    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER:
        vPos.x -= m_fOriginSize * 0.5f;
        origin = { vTextSize.x * 0.5f   , vTextSize.y * 0.5f };
        break;
    default:
        break;
    }

    origin.y -= m_pSpriteFont->GetLineSpacing() * 0.1f;

    m_pSpriteFont->DrawString(pBatch, pText, vPos, vColor,0.f, origin);

    return S_OK;
}

HRESULT CFont::Draw(SpriteBatch* pBatch, const TCHAR* pText, const Float2& vPosition, const Float2& vSize, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{
    Float2 vPos = vPosition;
    XMFLOAT2 origin{ 0.f,0.f };

    if (eType == FONT_RENDER_TYPE::DEFUALT)
    {
        m_pSpriteFont->DrawString(pBatch, pText, vPos, vColor, 0.f, origin, vSize);
        return S_OK;
    }

    Float2 vTextSize{};
    XMStoreFloat2(&vTextSize, m_pSpriteFont->MeasureString(pText));

    switch (eType)
    {
    case Engine::FONT_RENDER_TYPE::DEFUALT: break;

    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER_X:
        vPos.x -= m_fOriginSize * vSize.x * 0.5f;
        origin = { vTextSize.x * 0.5f  , 0.f };
        break;
    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER_Y:
        origin = { 0.f , vTextSize.y * 0.5f };
        break;
    case Engine::FONT_RENDER_TYPE::ALIGN_CENTER:
        vPos.x -= m_fOriginSize * vSize.x * 0.5f;
        origin = { vTextSize.x * 0.5f   , vTextSize.y * 0.5f };
        break;
    default:
        break;
    }

    origin.y -= m_pSpriteFont->GetLineSpacing() * 0.1f;


    m_pSpriteFont->DrawString(pBatch, pText, vPos, vColor, 0.f, origin, vSize);

    return S_OK;
}


CFont* CFont::Create(float fOriginSize, const wstring& wstrFontPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CFont* pFont = new CFont(pDevice, pContext);
    if (FAILED(pFont->Initialize(fOriginSize,wstrFontPath)))
    {
        Safe_Release(pFont);
        MSG_BOX("Font is failed to Create");
        return nullptr;
    }
    return pFont;
}

void CFont::Free()
{
    Safe_Delete(m_pSpriteFont);
}