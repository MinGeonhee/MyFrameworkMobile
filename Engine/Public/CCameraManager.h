#pragma once
#include "CBase.h"
class CCameraObjectBase;
class ENGINE_DLL CCameraManager final:public CBase
{
private:CCameraManager()=default;
public:
 ~CCameraManager()override=default;
 HRESULT Add_Camera(const std::wstring& strKey,UPDATE_MODE eMode,CCameraObjectBase* pCamera);
 HRESULT Remove_Camera(const std::wstring& strKey,UPDATE_MODE eMode);
 CCameraObjectBase* Get_Camera(const std::wstring& strKey,UPDATE_MODE eMode)const;
 HRESULT Switch_MainCamera(const std::wstring& strKey,UPDATE_MODE eMode);
 CCameraObjectBase* Get_MainCamera()const{return m_pMainCamera;}
 void Pre_Update(float dt);void Update(float dt);void Late_Update(float dt);HRESULT Render();void Resize(float w,float h);
 static CCameraManager* Create();void Free()override;
private:
 std::array<std::unordered_map<std::wstring,CCameraObjectBase*>,static_cast<size_t>(UPDATE_MODE::END)>m_Cameras;
 CCameraObjectBase*m_pMainCamera=nullptr;
};
