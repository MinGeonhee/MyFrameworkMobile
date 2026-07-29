#include "pch.h"
#include "CPlayerManager.h"
#include "CGameObject.h"
CPlayerManager* CPlayerManager::Create(){return new CPlayerManager{};}
HRESULT CPlayerManager::Add_Player(const std::wstring& k,CGameObject* p){if(k.empty()||!p||m_Players.count(k))return E_FAIL;p->AddRef();m_Players.emplace(k,p);return S_OK;}
CGameObject* CPlayerManager::Get_Player(const std::wstring& k)const{auto it=m_Players.find(k);return it!=m_Players.end()?it->second:nullptr;}
void CPlayerManager::Remove_Player(const std::wstring& k){auto it=m_Players.find(k);if(it!=m_Players.end()){it->second->Release();m_Players.erase(it);}}
void CPlayerManager::Clear_Player(){for(auto& p:m_Players)p.second->Release();m_Players.clear();}
void CPlayerManager::Free(){Clear_Player();CBase::Free();}
