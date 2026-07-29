#include "pch.h"
#include "CUIManager.h"
#include "CGameObject.h"
CUIManager* CUIManager::Create(){return new CUIManager{};}
HRESULT CUIManager::Ready_DefaultUI(){return S_OK;}
HRESULT CUIManager::Add_Canvas(uint32 iIndex,CGameObject* pCanvas){if(!pCanvas||m_Canvases.count(iIndex))return E_INVALIDARG;pCanvas->AddRef();m_Canvases.emplace(iIndex,pCanvas);return S_OK;}
CGameObject* CUIManager::Get_Canvas(uint32 iIndex)const{auto i=m_Canvases.find(iIndex);return i==m_Canvases.end()?nullptr:i->second;}
void CUIManager::Release_Canvas(uint32 iIndex){auto i=m_Canvases.find(iIndex);if(i==m_Canvases.end())return;i->second->Release();m_Canvases.erase(i);}
void CUIManager::Fade_In(float fTime){m_eFadeState=UI_FADE_STATE::FADE_IN;m_fFadeDuration=std::max(fTime,0.001f);m_fFadeElapsed=0.f;m_fFadeAlpha=1.f;}
void CUIManager::Fade_Out(float fTime){m_eFadeState=UI_FADE_STATE::FADE_OUT;m_fFadeDuration=std::max(fTime,0.001f);m_fFadeElapsed=0.f;m_fFadeAlpha=0.f;}
void CUIManager::Pre_Update(float dt){for(auto& p:m_Canvases)if(!p.second->Get_Dead())p.second->Pre_Update(dt);}
void CUIManager::Update(float dt){for(auto& p:m_Canvases)if(!p.second->Get_Dead())p.second->Update(dt);if(m_eFadeState!=UI_FADE_STATE::NONE){m_fFadeElapsed+=dt;float t=std::min(m_fFadeElapsed/m_fFadeDuration,1.f);m_fFadeAlpha=m_eFadeState==UI_FADE_STATE::FADE_IN?1.f-t:t;if(t>=1.f)m_eFadeState=UI_FADE_STATE::NONE;}}
void CUIManager::Late_Update(float dt){for(auto& p:m_Canvases)if(!p.second->Get_Dead())p.second->Late_Update(dt);}
HRESULT CUIManager::Render(){for(auto& p:m_Canvases)if(!p.second->Get_Dead()&&FAILED(p.second->Render()))return E_FAIL;return S_OK;}
void CUIManager::Free(){for(auto& p:m_Canvases)p.second->Release();m_Canvases.clear();CBase::Free();}
