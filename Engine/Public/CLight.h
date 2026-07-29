#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
class ENGINE_DLL CLight final:public CBase
{
private:CLight()=default;
public:~CLight()override=default;const Engine::LIGHT_DESC& Get_Desc()const{return m_Desc;}void Set_Active(bool a){m_Desc.isActive=a;}static CLight* Create(const Engine::LIGHT_DESC& d);void Free()override;
private:Engine::LIGHT_DESC m_Desc{};
};
