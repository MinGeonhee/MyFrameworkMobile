#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
class CLight;
class ENGINE_DLL CLightManager final:public CBase
{
private:CLightManager()=default;
public:~CLightManager()override=default;HRESULT Add_Light(const Engine::LIGHT_DESC& d);HRESULT Add_Light(CLight* l);CLight* Get_Light(LIGHT_TYPE t,uint32 index)const;const std::vector<CLight*>& Get_LightList(LIGHT_TYPE t)const;void Clear_Light();static CLightManager* Create();void Free()override;
private:std::array<std::vector<CLight*>,static_cast<size_t>(LIGHT_TYPE::END)>m_Lights;static const std::vector<CLight*> s_Empty;
};
