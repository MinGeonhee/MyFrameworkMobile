#pragma once
#include "CBase.h"
class CGameObject;
class ENGINE_DLL CUIManager final : public CBase
{
private: CUIManager() = default;
public:
    ~CUIManager() override = default;
    HRESULT Ready_DefaultUI();
    HRESULT Add_Canvas(uint32 iIndex, CGameObject* pCanvas);
    CGameObject* Get_Canvas(uint32 iIndex) const;
    void Release_Canvas(uint32 iIndex);
    void Fade_In(float fTime); void Fade_Out(float fTime);
    float Get_FadeAlpha() const { return m_fFadeAlpha; }
    void Pre_Update(float fTimeDelta); void Update(float fTimeDelta); void Late_Update(float fTimeDelta);
    HRESULT Render();
    static CUIManager* Create();
    void Free() override;
private:
    std::map<uint32,CGameObject*> m_Canvases;
    UI_FADE_STATE m_eFadeState=UI_FADE_STATE::NONE;
    float m_fFadeAlpha=0.f, m_fFadeDuration=0.f, m_fFadeElapsed=0.f;
};
