#include "CScreenFadeUI.h"
#include "CGameManager.h"
#include "CUI_Image.h"
#include "CUI_Animation.h"

CScreenFadeUI::CScreenFadeUI(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CUIObject(pDevice,pContext) ,m_pUIImageCom(nullptr), m_pUIAnimationCom(nullptr)
{

}


HRESULT	CScreenFadeUI::Initialize()
{
    float OriginWinSizeX = static_cast<float>(m_pGameManager->Get_DefaultWinSizeX());
    float OriginWinSizeY = static_cast<float>(m_pGameManager->Get_DefaultWinSizeY());

    /* BackGroundLayer UIObject 생성 */
    CUIObject::UIOBJECT_DESC tUIObjectDesc{};
    tUIObjectDesc.wstrName = L"ScreenFadeUI";
    tUIObjectDesc.eUIDepth = UI_DEPTH::DEPTH_1;
    tUIObjectDesc.vSize = { OriginWinSizeX,  OriginWinSizeY };

    if (FAILED(__super::Initialize(&tUIObjectDesc)))
        return E_FAIL;

    if (FAILED(Ready_Component()))
        return E_FAIL;

    return S_OK;
}

HRESULT	CScreenFadeUI::Ready_Component()
{
    CUI_Image::UI_IMAGE_DESC tUIImageDesc{};
    tUIImageDesc.wstrFindTextureKey = L"ScreenFade";

    if (FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UIImage", L"UICom_Image", &tUIImageDesc, reinterpret_cast<CUIComponent**>(&m_pUIImageCom))))
        return E_FAIL;


    /* 뒤에 이미지를 FadeIn 시키기위한 Animation */
    CUI_Animation::UI_Aniamtion_Desc tUIAnimationDesc{};
    tUIAnimationDesc.pOwnerUITransform = m_pUITransformCom;
    tUIAnimationDesc.pOwnerUIImageCom = m_pUIImageCom;

    if (FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UIAnimation", L"UICom_Animation", &tUIAnimationDesc, reinterpret_cast<CUIComponent**>(&m_pUIAnimationCom))))
        return E_FAIL;

    return S_OK;
}

void	CScreenFadeUI::Pre_Update(CCanvas* pCanvas, float DT)
{
    __super::Pre_Update(pCanvas,DT);
}

void	CScreenFadeUI::Update(CCanvas* pCanvas, float DT)
{
    __super::Update(pCanvas, DT);
}

void	CScreenFadeUI::Late_Update(CCanvas* pCanvas, float DT)
{
    __super::Late_Update(pCanvas, DT);
}

HRESULT	CScreenFadeUI::Render(CCanvas* pCanvas)
{
    __super::Render(pCanvas);

    return S_OK;
}

void CScreenFadeUI::Fade_In(float fTime)
{
    NULLPTR_RETURN_NULL(m_pUIAnimationCom);
    NULLPTR_RETURN_NULL(m_pUIImageCom);

    m_pUIAnimationCom->Fade_In(fTime);
}

void CScreenFadeUI::Fade_Out(float fTime)
{
    NULLPTR_RETURN_NULL(m_pUIAnimationCom);
    NULLPTR_RETURN_NULL(m_pUIImageCom);

    m_pUIAnimationCom->Fade_Out(fTime);
}

CScreenFadeUI* CScreenFadeUI::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{

    CScreenFadeUI* pUIObject = new CScreenFadeUI(pDevice, pContext);

    if (FAILED(pUIObject->Initialize()))
    {
        Safe_Release(pUIObject);
        MSG_BOX("ScreenFadeUI is failed to Create");
        return nullptr;
    }

    return pUIObject;

}
void CScreenFadeUI::Free()
{
    __super::Free();


    Safe_Release(m_pUIAnimationCom);
    Safe_Release(m_pUIImageCom);
}