#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
class ENGINE_DLL CCell final:public CBase{private:CCell()=default;public:~CCell()override=default;const Engine::CELL_DESC& Get_Desc()const{return m_Desc;}static CCell* Create(const Engine::CELL_DESC& d);void Free()override;private:Engine::CELL_DESC m_Desc{};};
