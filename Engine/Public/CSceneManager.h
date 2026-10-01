#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CScene;
class CGameManager;
using SceneFactory = std::function<CScene* (const CreateScene_Desc&)>;
//using ChangeSceneFuntion = std::function<HRESULT()>


class CSceneManager :  public CBase
{
private:
	explicit		CSceneManager();
	virtual			~CSceneManager();
private:
	HRESULT			Initialize();
public:
	HRESULT			Register_SceneFactory(uint32 iSceneID ,const SceneFactory& SceneCreateFuntion);
	void			RequestChangeScene(const CreateScene_Desc& tDesc);
	HRESULT			ExecutePendingSceneChanges();
	const uint32	GetCurSceneID();
	CScene*			GetCurScene();
public:
	void			Pre_Update(float DT);
	void			Update(float DT);
	void			Late_Update(float DT);
	void			Add_RenderGroup();
	HRESULT			Render();
public:
	/* Get SceneStack */
	vector<CScene*>&		GetStackScene();
	/* Get CurSceneID */
	uint32					GetCurrentSceneID();

private:

	HRESULT					Push_Scene(CScene* pChangeScene);
	HRESULT					Replace_Scene(CScene* pChangeScene);
	HRESULT					Clear_Scene(CScene* pChangeScene);

	HRESULT					TemporarySceneRelease();
	HRESULT					PopTopScene();
	HRESULT					ChangeScene(const CreateScene_Desc& tCreateScene_Desc);
	HRESULT					ChangeScene(bool isPop);

	void					SaveTexturePreviousScene();
	CScene*					MakeSceneByFactory(const CreateScene_Desc& tCreateScene_Desc);
	const SceneFactory*		Find_SceneFactory(uint32 Key);

private:
	//스택으로 사용할 vector Scene* 배열
	vector<CScene*>			m_stackScene;
	/* 정리가될 Scene 담아두기.. 바로 삭제x LateUpdate 끝자락이나 Pre_Update돌기전 비워주기 */
	queue<std::function<HRESULT()>>	m_queueSceneChange;
	//SceneFactory
	unordered_map<uint32, SceneFactory>	m_umapSceneFactory;

	//받아서 사용할 매니저
	CGameManager*			m_pGameManager;
public:
	static	CSceneManager*	Create();
	virtual	void			Free()override;
};

NS_END