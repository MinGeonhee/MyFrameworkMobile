#include "CScene.h"
#include "CGameManager.h"


CScene::CScene(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext),m_tSceneInfo{} , m_pGameManager(CGameManager::GetInstance())
	, m_tCreateSceneInfo{}, m_isPause(false)
{
	Safe_AddRef(m_pGameManager);
}

CScene::~CScene()
{

}



void CScene::OnEnter()
{
	m_pGameManager->CollisionManager_Clear_Collision();
	m_pGameManager->LightManager_Clear_Light();

	Ready_CollisionTable();
	m_pGameManager->GameObjectManager_Add_ColliderGroup(m_tSceneInfo.iSceneID);
	Ready_Light();

	m_pGameManager->UIManager_Fade_Out(2.0f);
}

void CScene::OnExit()
{
	m_pGameManager->UIManager_Fade_In(2.0f);
}
void CScene::OnPause()
{
	m_pGameManager->UIManager_Fade_In(1.0f);
}
void CScene::OnResume()
{
	m_pGameManager->UIManager_Fade_Out(1.0f);
}

void	CScene::Pre_Update(float DT)
{
	return m_pGameManager->GameObjectManager_Pre_Update(m_tSceneInfo.iSceneID, DT);
}
void	CScene::Update(float DT)
{
	return m_pGameManager->GameObjectManager_Update(m_tSceneInfo.iSceneID, DT);
}
void	CScene::Late_Update(float DT)
{
	return m_pGameManager->GameObjectManager_Late_Update(m_tSceneInfo.iSceneID, DT);
}

void CScene::Add_RenderGroup()
{
	m_pGameManager->GameObjectManager_Add_RenderGroup(m_tSceneInfo.iSceneID);
	return;
}


void CScene::Render_WindowText_Scene_Name(HWND hWnd, const TCHAR* sText)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(sText);
}



HRESULT CScene::Render()
{
	return S_OK;
}



HRESULT CScene::Initialize(const CreateScene_Desc& tCreateSceneDesc)
{
	m_tCreateSceneInfo = tCreateSceneDesc;

	return S_OK;
}

void CScene::Free()
{
	__super::Free();

	/* Scene::Exit() */
	this->OnExit();

	Safe_Release(m_pGameManager);
}
