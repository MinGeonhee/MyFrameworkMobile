#include "pch.h"
#include "CLight.h"
CLight* CLight::Create(const Engine::LIGHT_DESC& d){auto* p=new CLight{};p->m_Desc=d;return p;}void CLight::Free(){CBase::Free();}
