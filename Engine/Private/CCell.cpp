#include "pch.h"
#include "CCell.h"
CCell* CCell::Create(const Engine::CELL_DESC& d){auto* p=new CCell{};p->m_Desc=d;return p;}void CCell::Free(){CBase::Free();}
