#include "CSceneManager.h"
#include "CGameManager.h"
#include "CScene.h"

USING(Engine)

CSceneManager::CSceneManager()
	: m_pGameManager(CGameManager::GetInstance()), m_umapSceneFactory{}
{
	m_stackScene.reserve(50);
	Safe_AddRef(m_pGameManager);
}

CSceneManager::~CSceneManager()
{
}

HRESULT	CSceneManager::Initialize()
{


	return S_OK;
}

HRESULT CSceneManager::ChangeScene(const CreateScene_Desc& tCreateScene_Desc)
{
	if (tCreateScene_Desc.eIsPop == SCENE_ISPOP::YES)
		return PopTopScene();

	/* 새로들어온 Scene의 RenderFlag 타입이 이전 Scene을 렌더 해주는 기능이라면 함수 호출 */
	if (tCreateScene_Desc.isNeedSceneOverLay == true)
		SaveTexturePreviousScene();

	CScene* pChangeScene = MakeSceneByFactory(tCreateScene_Desc); NULLPTR_RETURN_EFAIL(pChangeScene);
	SCENE_INFO tChangeSceneInfo = pChangeScene->GetSceneInfo();



	/* 임시 객채 Scene먼저 Pop해주기 */
	TemporarySceneRelease();

	switch (pChangeScene->GetSceneInfo().eStackType)
	{
	case Engine::SCENE_STACK_TYPE::PUSH:
		Push_Scene(pChangeScene);
		break;
	case Engine::SCENE_STACK_TYPE::REPLACE:
		Replace_Scene(pChangeScene);
		break;
	case Engine::SCENE_STACK_TYPE::CLEAR:
		Clear_Scene(pChangeScene);
		break;
	default:
		break;
	}

	return S_OK;
}

HRESULT CSceneManager::ChangeScene(bool isPop)
{
	return S_OK;
}

HRESULT CSceneManager::ExecutePendingSceneChanges()
{
	if (m_queueSceneChange.empty())
		return S_OK;

	/* 한프레임에 한번만 처리 */

	if (FAILED(m_queueSceneChange.front()()))
	{
		MSG_BOX("Scene Change is Failed");
	}
	else
	{ 
		//m_pGameManager->PipeLine_Clear_PipeLine();
		m_queueSceneChange.pop();
		

		/* ChangeScene이 불리고나서...가장 위의 씬의 OnEnter가 불리게 된다 */
		if (m_stackScene.back()->Get_IsPause())
			m_stackScene.back()->OnResume();
		else
		{
			m_stackScene.back()->OnEnter();
			m_stackScene.back()->Set_IsPause(false);
		}
	}

	return S_OK;
}

HRESULT CSceneManager::PopTopScene()
{
	if (m_stackScene.empty())
		return E_FAIL;

	CScene* pScene{ nullptr };

	pScene = this->m_stackScene.back();
	m_stackScene.pop_back();

	/* 맨위에 있는 Scene 을 삭제예정 Queue에 넣어주기 */
	m_pGameManager->ChangeScene_Clear_CurSceneObject(pScene->GetSceneID());
	pScene->OnExit();
	Safe_Release(pScene);

	return S_OK;
}

HRESULT CSceneManager::Register_SceneFactory(uint32 iSceneID, const SceneFactory& SceneCreateFuntion)
{
	const SceneFactory* pSceneFactory{ nullptr };

	pSceneFactory = Find_SceneFactory(iSceneID);

	if (pSceneFactory != nullptr) return E_FAIL;

	m_umapSceneFactory.emplace(iSceneID, SceneCreateFuntion);

	return S_OK;
}

const uint32 CSceneManager::GetCurSceneID()
{
	return m_stackScene.back()->GetSceneID();
}

CScene* CSceneManager::GetCurScene()
{
	if (m_stackScene.empty()) return nullptr;

	return m_stackScene.back();
}

HRESULT CSceneManager::Push_Scene(CScene* pChangeScene)
{	
	NULLPTR_RETURN_EFAIL(pChangeScene);

	/* 이전 Scene 잠시 pause */
	if (!m_stackScene.empty())
	{
		m_stackScene.back()->OnPause();
		m_stackScene.back()->Set_IsPause(true);
	}
	m_stackScene.push_back(pChangeScene);
	return S_OK;
}

HRESULT CSceneManager::Replace_Scene(CScene* pChangeScene)
{
	NULLPTR_RETURN_EFAIL(pChangeScene);

	if (!m_stackScene.empty())
		PopTopScene();

	m_stackScene.push_back(pChangeScene);

	return S_OK;
}

HRESULT CSceneManager::Clear_Scene(CScene* pChangeScene)
{
	NULLPTR_RETURN_EFAIL(pChangeScene);

	for (auto i = 0; i < m_stackScene.size(); ++i)
		PopTopScene();

	return S_OK;
}

HRESULT CSceneManager::TemporarySceneRelease()
{
	if (m_stackScene.empty())
		return S_OK;
	/* 임시 Scene은 순회를 돌면서 삭제할 이유가없다 Scene이 Change가 불릴 때마다 삭제가 되기 때문에 Scene에서 임시 Scene은 1개만 존재할거다 아마도? */
	if (m_stackScene.back()->GetSceneInfo().ePopFlag & SCENE_POPFLAG::Scene_Pop_Temporary)
		PopTopScene();

	return S_OK;
}

void CSceneManager::SaveTexturePreviousScene()
{
	m_pGameManager->ScreenShotManager_ScreenShot(true, NULLPTR);
}


CScene* CSceneManager::MakeSceneByFactory(const CreateScene_Desc& tCreateScene_Desc)
{
	const SceneFactory* pFactory = Find_SceneFactory(tCreateScene_Desc.iMakeSceneNum);
	NULLPTR_RETURN_NULLPTR(pFactory)

	return pFactory->operator()(tCreateScene_Desc);
}

const SceneFactory* CSceneManager::Find_SceneFactory(uint32 Key)
{
	auto iter = m_umapSceneFactory.find(Key);

	if (iter == m_umapSceneFactory.end())
		return nullptr;

	return &(iter->second);
}

void CSceneManager::RequestChangeScene(const CreateScene_Desc& tDesc)
{
	/* Queue에 요청을 넣기 */
	m_queueSceneChange.push([this, tDesc]() -> HRESULT { return ChangeScene(tDesc);});
	/* 중복으로 들어오는 SceneChange막기*/
}

CSceneManager* CSceneManager::Create()
{
	CSceneManager* pSceneManager = new CSceneManager();

	INIT_FAILED_CEHCK(pSceneManager);

	return pSceneManager;
}

void	CSceneManager::Pre_Update(float DT)
{
	if (m_stackScene.empty())
		return;

	NULLPTR_RETURN_NULL(m_stackScene.back());


	m_stackScene.back()->Pre_Update(DT);

	return;
}

void	CSceneManager::Update(float DT)
{
	if (m_stackScene.empty())
		return;


	NULLPTR_RETURN_NULL(m_stackScene.back());


	m_stackScene.back()->Update(DT);

	return;
}

void	CSceneManager::Late_Update(float DT)
{
	if (m_stackScene.empty())
		return;

	NULLPTR_RETURN_NULL(m_stackScene.back());

	m_stackScene.back()->Late_Update(DT);
}

void CSceneManager::Add_RenderGroup()
{
	if (m_stackScene.empty())
		return;

	NULLPTR_RETURN_NULL(m_stackScene.back());

	m_stackScene.back()->Add_RenderGroup();
}


HRESULT			CSceneManager::Render()
{
	if( !m_stackScene.empty())
		m_stackScene.back()->Render();
	return S_OK;
}


vector<CScene*>& CSceneManager::GetStackScene()
{
	return m_stackScene;
}
uint32 CSceneManager::GetCurrentSceneID()
{
	return m_stackScene.back()->GetSceneID();
}



void CSceneManager::Free()
{
	__super::Free();

	uint32 Count{(uint32)m_stackScene.size() };
	 
	for (uint32 i = 0; i < Count ; ++i)
	{
		Safe_Release(m_stackScene.back());
		m_stackScene.pop_back();
	}


	m_umapSceneFactory.clear(); // 모든 SceneFactory 해제

	Safe_Release(m_pGameManager);
}






