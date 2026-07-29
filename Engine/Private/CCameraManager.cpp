#include "pch.h"
#include "CCameraManager.h"
#include "CCameraObjectBase.h"
CCameraManager* CCameraManager::Create(){return new CCameraManager{};}
HRESULT CCameraManager::Add_Camera(const std::wstring&k,UPDATE_MODE m,CCameraObjectBase*c){size_t i=static_cast<size_t>(m);if(i>=m_Cameras.size()||k.empty()||!c||m_Cameras[i].count(k))return E_FAIL;c->AddRef();m_Cameras[i].emplace(k,c);if(!m_pMainCamera)m_pMainCamera=c;return S_OK;}
HRESULT CCameraManager::Remove_Camera(const std::wstring&k,UPDATE_MODE m){size_t i=static_cast<size_t>(m);if(i>=m_Cameras.size())return E_INVALIDARG;auto it=m_Cameras[i].find(k);if(it==m_Cameras[i].end())return E_FAIL;if(m_pMainCamera==it->second)m_pMainCamera=nullptr;it->second->Release();m_Cameras[i].erase(it);return S_OK;}
CCameraObjectBase* CCameraManager::Get_Camera(const std::wstring&k,UPDATE_MODE m)const{size_t i=static_cast<size_t>(m);if(i>=m_Cameras.size())return nullptr;auto it=m_Cameras[i].find(k);return it==m_Cameras[i].end()?nullptr:it->second;}
HRESULT CCameraManager::Switch_MainCamera(const std::wstring&k,UPDATE_MODE m){auto*c=Get_Camera(k,m);if(!c)return E_FAIL;m_pMainCamera=c;return S_OK;}
void CCameraManager::Pre_Update(float dt){for(auto&map:m_Cameras)for(auto&p:map)if(!p.second->Get_Dead())p.second->Pre_Update(dt);}
void CCameraManager::Update(float dt){for(auto&map:m_Cameras)for(auto&p:map)if(!p.second->Get_Dead())p.second->Update(dt);}
void CCameraManager::Late_Update(float dt){for(auto&map:m_Cameras)for(auto&p:map)if(!p.second->Get_Dead())p.second->Late_Update(dt);}
HRESULT CCameraManager::Render(){return m_pMainCamera?m_pMainCamera->Set_GPU_BeforeRender():S_FALSE;}
void CCameraManager::Resize(float w,float h){for(auto&map:m_Cameras)for(auto&p:map)p.second->Resize(w,h);}
void CCameraManager::Free(){for(auto&map:m_Cameras){for(auto&p:map)p.second->Release();map.clear();}m_pMainCamera=nullptr;CBase::Free();}
