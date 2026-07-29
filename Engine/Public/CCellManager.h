#pragma once
#include "CBase.h"
class CCell;
class ENGINE_DLL CCellManager final:public CBase
{
private:CCellManager()=default;
public:~CCellManager()override=default;HRESULT Add_Cells(const std::wstring& key,const std::vector<CCell*>& cells);const std::vector<CCell*>& Get_Cells(const std::wstring& key)const;bool Get_IsCellRender()const{return m_isCellRender;}void Set_IsCellRender(bool b){m_isCellRender=b;}static CCellManager* Create();void Free()override;
private:std::unordered_map<std::wstring,std::vector<CCell*>>m_Cells;bool m_isCellRender=false;static const std::vector<CCell*>s_Empty;
};
