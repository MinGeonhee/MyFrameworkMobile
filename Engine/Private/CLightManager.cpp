#include "pch.h"
#include "CLightManager.h"
#include "CLight.h"
const std::vector<CLight*> CLightManager::s_Empty{};CLightManager* CLightManager::Create(){return new CLightManager{};}
HRESULT CLightManager::Add_Light(const Engine::LIGHT_DESC& d){auto* l=CLight::Create(d);if(!l)return E_FAIL;return Add_Light(l);}
HRESULT CLightManager::Add_Light(CLight* l){if(!l)return E_INVALIDARG;size_t i=static_cast<size_t>(l->Get_Desc().eType);if(i>=m_Lights.size())return E_INVALIDARG;m_Lights[i].push_back(l);return S_OK;}
CLight* CLightManager::Get_Light(LIGHT_TYPE t,uint32 index)const{size_t i=static_cast<size_t>(t);return i<m_Lights.size()&&index<m_Lights[i].size()?m_Lights[i][index]:nullptr;}
const std::vector<CLight*>& CLightManager::Get_LightList(LIGHT_TYPE t)const{size_t i=static_cast<size_t>(t);return i<m_Lights.size()?m_Lights[i]:s_Empty;}
void CLightManager::Clear_Light(){for(auto& v:m_Lights){for(auto* l:v)l->Release();v.clear();}}
void CLightManager::Free(){Clear_Light();CBase::Free();}
