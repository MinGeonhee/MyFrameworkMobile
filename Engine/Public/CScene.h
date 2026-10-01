#pragma once
#include "CBase.h"

/* Scene 을 Stack으로 쌓아서 관리한다는 의미는 이전 Scene의 Update는 필요없다는 구조가 맞는구조이다 흔들리지말자 */
/* UI Scene 같은 경우는 뒤에 부분이 Scene이 Blur 처리가 되기 때문에 SceneChange를 하는 순가 Texture2D를 이용하여 화면을 찍어내고 난뒤 */
/* 찍어낸 화면을 저장해서 한번 ResourceManager에 저장을 시켜준뒤 RenderManager에서 SceneObject들이 Render하기전에 미리 Render를 해준다.*/
/* 이렇게 되면 스택 Scene 구조에서 이전 Scene에 대한 정보는 필요하다고 한들 Update를 돌릴 필요가 없다. */
/* 만약 Render가 필요없는 Scene이다? 그런 Scnee은 존재하지 않는다....완벽한 Engine은 있을 수 없다. */
/* 근데 만약 SceneRenderFlag 에 내 밑에 Scene을 Render 해준다는 개념이 있다면 Scene을 순회하여 Previous Render타입이 아닌 Scene이 있을 떄 까지 내려준다*/
/* 그런데도 그게없다면 그건모르겠다 아마 있지않을까 왠만해서는 */
/* 그냥 백퍼퍼 색깔나와야지뭐 */

NS_BEGIN(Engine)



enum ScenePopFlag
{
	Scene_Pop_Default = 0,	// Default
	Scene_Pop_Temporary = ( 1 << 0), // 임시 Scene   Ex)) Loading 
};
using SCENE_POPFLAG = ScenePopFlag;

enum SceneRenderFlag
{
	Scene_Render_OnlyThisScene = 0 ,
	Scene_Render_WithPreviousScene = (1 << 0),
};

using SCENE_RENDERFLAG = SceneRenderFlag;

struct Scene_Info
{
	uint32					iSceneID{ 0 };
	SCENE_STACK_TYPE		eStackType{SCENE_STACK_TYPE::NONE};
	SCENE_POPFLAG			ePopFlag{ Scene_Pop_Default };
	SCENE_RENDERFLAG		eRenderFlag{ Scene_Render_OnlyThisScene };
};

using SCENE_RENDER_FALG = SceneRenderFlag;
using SCENE_INFO = Scene_Info;

class CGameManager;

class ENGINE_DLL CScene  abstract :  public CBase
{
protected:
	explicit CScene(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual	 ~CScene();
protected:
	void					Render_WindowText_Scene_Name(HWND hWnd,const TCHAR* sText);
public:
	virtual const uint32&   GetSceneID() const { return m_tSceneInfo.iSceneID; }
	virtual	void			Pre_Update(float DT);
	virtual	void			Update(float DT);
	virtual	void			Late_Update(float DT);
	virtual	void			Add_RenderGroup();
	virtual HRESULT			Render();
public:
	virtual	void			OnEnter();
	virtual	void			OnExit();
	virtual	void			OnPause();
	virtual	void			OnResume();
public:
	virtual void			Ready_CollisionTable()PURE;
	virtual void			Ready_Light() {};
public:
	void					Set_IsPause(bool isPause) { m_isPause = isPause; }
public:
	bool					Get_IsPause() const { return m_isPause; }
	const SCENE_INFO&		GetSceneInfo() const { return m_tSceneInfo; }
	const CreateScene_Desc& GetCreateSceneDesc() const { return m_tCreateSceneInfo;}
#pragma region For ImGui Debug
public:
	virtual	string			GetSceneName() PURE;
#pragma endregion
protected:
	virtual	HRESULT			Initialize(const CreateScene_Desc& tCreateSceneDesc);
protected:
	bool					m_isPause;

	CreateScene_Desc		m_tCreateSceneInfo;

	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
	CGameManager*			m_pGameManager;
protected:
	SCENE_INFO				m_tSceneInfo;
public:
	virtual	void			Free()override;
};

NS_END;
