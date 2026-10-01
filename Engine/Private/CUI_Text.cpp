#include "CUI_Text.h"
#include "CGameManager.h"
#include "CFont.h"
#include "CUI_Animation.h"
#include "CCanvas.h"

CUI_Text::CUI_Text(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CUIObject(pDevice, pContext), m_pUIAnimationCom(nullptr), m_vColor{0.f,0.f,0.f,1.f}
    , m_pSpriteBatch{nullptr} , m_eFontRenderType(FONT_RENDER_TYPE::DEFUALT)
    ,m_pFont(nullptr)
    , m_wstrText{}
{
}

HRESULT CUI_Text::Initialize(UI_TEXT_DESC* pDesc)
{
    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_pSpriteBatch = new SpriteBatch(m_pDeviceContext.Get());
    m_pFont = m_pGameManager->FontMananger_Get_Font(pDesc->wstrFindFontKey);
    NULLPTR_RETURN_EFAIL(m_pFont);
    NULLPTR_RETURN_EFAIL(m_pSpriteBatch);

    m_wstrText = pDesc->wstrText;
    m_vColor = pDesc->vColor;


    m_eFontRenderType = pDesc->eFontRenderType;

    if (FAILED(Ready_Component(pDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT	CUI_Text::Ready_Component(UI_TEXT_DESC* pDesc)
{
    CUI_Animation::UI_ANIAMTION_DESC tUIAnimDesc{};
    tUIAnimDesc.pOwnerUIImageCom     = nullptr;
    tUIAnimDesc.pOwnerUITransform    = m_pUITransformCom;

    if (FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UIAnimation", L"Com_UIAnimation", &tUIAnimDesc, reinterpret_cast<CUIComponent**>(&m_pUIAnimationCom))))
        return E_FAIL;


    return S_OK;
}

void CUI_Text::Pre_Update(CCanvas* pCanvas, float DT)
{
    __super::Pre_Update(pCanvas, DT);
}

void CUI_Text::Update(CCanvas* pCanvas, float DT)
{
    __super::Update(pCanvas, DT);
}

void CUI_Text::Late_Update(CCanvas* pCanvas, float DT)
{
    for (auto& UICom : m_vecUIComponent)
        if (UICom)
            if (UICom->Get_IsActive())
                UICom->Late_Update(DT);


    m_pUITransformCom->Update_World(pCanvas);
}

HRESULT CUI_Text::Render(CCanvas* pCanvas)
{
    if (FAILED(__super::Render(pCanvas)))
        return E_FAIL;

    VIEWPORT_DESC tViewPort = pCanvas->Get_CanvasViewPort();
    XMMATRIX matWorld = XMLoadFloat4x4(m_pUITransformCom->Get_WorldPtr());
    XMMATRIX matProj = XMLoadFloat4x4(pCanvas->Get_CanvasMatrixPtr());
    XMMATRIX matView = XMMatrixIdentity();

    Vector4 vPos = XMVector3Project( matWorld.r[3] ,0.f , 0.f , tViewPort.Width, tViewPort.Height, tViewPort.MinDepth, tViewPort.MaxDepth,
        matProj,matView , XMMatrixIdentity());

    m_pSpriteBatch->Begin();

    m_pFont->Draw( m_pSpriteBatch , m_wstrText.c_str() , Float2( XMVectorGetX(vPos), XMVectorGetY(vPos)), m_pUITransformCom->Get_Size(), XMLoadFloat4(&m_vColor),m_eFontRenderType);

    m_pSpriteBatch->End();

    return S_OK;
}

CUI_Text* CUI_Text::Create(UI_TEXT_DESC* pDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CUI_Text* pText = new CUI_Text(pDevice, pContext);

    if (FAILED(pText->Initialize(pDesc)))
    {
        Safe_Release(pText);
        MSG_BOX(" UI Text is failed to Create");
        return nullptr;
    }

    return pText;
}

void CUI_Text::Free()
{
    __super::Free();


    Safe_Delete(m_pSpriteBatch);
    Safe_Release(m_pUIAnimationCom);
}
