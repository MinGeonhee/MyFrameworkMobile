#pragma once
#include "CBase.h"
#include "CDefaultCameraObject.h"
#include "CUICameraObject.h"

namespace Engine
{
	class CCameraObjectBase;
	class CGameObject;
	class CGameManager;
}


NS_BEGIN(Engine)


class CCameraManager :    public CBase
{
	using SwitchMainCameraQueue = queue<std::function<HRESULT()>>;
private:
	explicit CCameraManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext);
	virtual ~CCameraManager() {};
private:
	HRESULT							Initialize();
public:
	void							Pre_Update(float DT);
	void							Update(float DT);
	void							Late_Update(float DT);
	HRESULT							Render();
#pragma region 봉인된 기능
private:
	void							Requset_SwitchMainCamera(CONST wstring& strKey, UPDATE_MODE eUpdateMode);
	void							ExecutePending_SwitchMainCamera();
	HRESULT							Switch_MainCamera(CONST wstring& strKey , UPDATE_MODE eUpdateMode);
	HRESULT							Set_MainCameraTarget(CGameObject* pGameObject);
	HRESULT							Set_SubCameraTarget(const wstring& strKey, UPDATE_MODE eUpdateMode, CCameraObjectBase* pGameObject);

#pragma endregion
public:
	HRESULT							MakeDefaultCamera(CONST CDefaultCameraObject::DefaultCameraObject_Dest* pDefaultCameraDest);
	HRESULT							MakeDefaultUICamera(CONST CUICameraObject::UICameraObject_Desc* pUICameraDest);
public: 
	HRESULT							Add_Camera(const wstring& wstrKey,UPDATE_MODE eUpdateMode ,CCameraObjectBase* pCamera);
	HRESULT							Remove_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode);

	/* Get */
	const array<unordered_map<wstring,	CCameraObjectBase*>, Cast_Uint(UPDATE_MODE::END) >& Get_CameraList() { return m_umapCamera; }
	CCameraObjectBase*					Get_Camera(CONST wstring& wstrKey,UPDATE_MODE eUpdateMode);
	CCameraObjectBase*					Get_CurRenderingCamera() { NULLPTR_RETURN_NULLPTR(m_pCurRenderingCamera); return m_pCurRenderingCamera; }
	const vector<CCameraObjectBase*>&	Get_RenderCamera() const { return m_vecRenderCamera;}
public:
	void							Set_CameraRenderActive(const wstring& wstrKey, UPDATE_MODE eUpdateMode, bool isRednerActive);
private:
	CCameraObjectBase*				Find_Camera( const wstring& wstrKey , UPDATE_MODE eUpdateMode);
	void							Add_And_Sort_CameraUpdate(float DT);
	void							Add_And_Sort_CameraRender();

private:
	CCameraObjectBase*						m_pMainCamera;
	CCameraObjectBase*						m_pCurRenderingCamera;
	array <unordered_map<wstring, CCameraObjectBase*>, Cast_Uint(UPDATE_MODE::END) >	m_umapCamera;

	vector<CCameraObjectBase*>				m_vecUpdateCamera;
	vector<CCameraObjectBase*>				m_vecRenderCamera;
	
	SwitchMainCameraQueue					m_queueSwitchMainCamera;
private:
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
private:
	CGameManager*							m_pGameManager;
public:
	static	CCameraManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext);
	virtual	void			Free() override;
};

NS_END
