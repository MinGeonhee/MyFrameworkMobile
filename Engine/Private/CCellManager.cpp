#include "pch.h"
#include "CCellManager.h"
#include "CCell.h"
const std::vector<CCell*> CCellManager::s_Empty{};CCellManager* CCellManager::Create(){return new CCellManager{};}
HRESULT CCellManager::Add_Cells(const std::wstring& k,const std::vector<CCell*>& cells){if(k.empty()||m_Cells.count(k))return E_FAIL;auto& dst=m_Cells[k];for(auto* c:cells){if(!c)continue;c->AddRef();dst.push_back(c);}return S_OK;}
const std::vector<CCell*>& CCellManager::Get_Cells(const std::wstring& k)const{auto it=m_Cells.find(k);return it!=m_Cells.end()?it->second:s_Empty;}
void CCellManager::Free(){for(auto& p:m_Cells)for(auto* c:p.second)c->Release();m_Cells.clear();CBase::Free();}
