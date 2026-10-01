#include "CUI_Quad.h"
#include "CUI_Animation.h"
#include "CUI_Image.h"
#include "CGameManager.h"


CUI_Quad::CUI_Quad(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CUIObject(pDevice, pContext)
    , m_pUIImageCom(nullptr) , m_pUIAnimationCom(nullptr)
{
}

HRESULT CUI_Quad::Initialize(UI_QUAD_DESC* pDesc)
{
    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;


    if (FAILED(Ready_Component(pDesc)))
        return E_FAIL;

    return S_OK;
}

void  CUI_Quad::Set_TextureIndex(int32 iIndex)
{
    m_pUIImageCom->Set_TextureIndex(iIndex);
}

void CUI_Quad::Set_Alpha(float fAlpha)
{
    m_pUIImageCom->Set_Aplha(fAlpha);
}

int32 CUI_Quad::Get_TextureIndex() const
{
    return m_pUIImageCom->Get_TextrueIndex();
}

HRESULT CUI_Quad::Ready_Component(UI_QUAD_DESC* pDesc)
{
    CUIComponent* pUICom{ nullptr };

    CUI_Image::UI_IMAGE_DESC tUIImageDesc{};
    tUIImageDesc.wstrFindTextureKey = pDesc->wstrTextureKey;
    tUIImageDesc.iTextureIndex = pDesc->iTextureIndex;

    if (FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UIImage", L"Com_UIImage", &tUIImageDesc, reinterpret_cast<CUIComponent**>(&m_pUIImageCom))))
        return E_FAIL;

    CUI_Animation::UI_ANIAMTION_DESC tUIAnimDesc{};
    tUIAnimDesc.pOwnerUIImageCom = m_pUIImageCom;
    tUIAnimDesc.pOwnerUITransform = m_pUITransformCom;
    
    if (FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UIAnimation", L"Com_UIAnimation", &tUIAnimDesc, reinterpret_cast<CUIComponent**>(&m_pUIAnimationCom))))
        return E_FAIL;

    m_pUIImageCom->Set_TextureIndex(pDesc->iTextureIndex);

    return S_OK;
}

void CUI_Quad::Pre_Update(CCanvas* pCanvas, float DT)
{
    __super::Pre_Update(pCanvas, DT);

    return;
}

void CUI_Quad::Update(CCanvas* pCanvas, float DT)
{
    __super::Update(pCanvas, DT);
    return;
}

void CUI_Quad::Late_Update(CCanvas* pCanvas, float DT)
{
    __super::Late_Update(pCanvas, DT);
    return;
}

HRESULT CUI_Quad::Render(CCanvas* pCanvas)
{
    /* 여기서 프레임 렌더 */
    __super::Render(pCanvas);

    return S_OK;
}

CUI_Quad* CUI_Quad::Create(UI_QUAD_DESC* pDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CUI_Quad* pUI = new CUI_Quad(pDevice, pContext);

    if (FAILED(pUI->Initialize(pDesc)))
    {
        Safe_Release(pUI);

        MSG_BOX("UI Frame is failed To Create");

        return nullptr;
    }
    return pUI;
}



void CUI_Quad::Free()
{
    __super::Free();

    Safe_Release(m_pUIImageCom);
    Safe_Release(m_pUIAnimationCom);
    return;
}