#include "pch.h"
#include "CScene.h"
#include "CGameManager.h"
CScene::CScene(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}, m_pDeviceContext{pDeviceContext}, m_pGameManager{CGameManager::GetInstance()}
{ if (m_pGameManager) m_pGameManager->AddRef(); }
HRESULT CScene::Initialize(const Engine::CREATE_SCENE_DESC& CreateDesc, const Engine::SCENE_INFO& SceneInfo)
{ m_tCreateSceneDesc=CreateDesc; m_tSceneInfo=SceneInfo; return S_OK; }
void CScene::Pre_Update(float dt) { if(m_pGameManager) m_pGameManager->GameObjectManager_Pre_Update(m_tSceneInfo.iSceneID,dt); }
void CScene::Update(float dt) { if(m_pGameManager) m_pGameManager->GameObjectManager_Update(m_tSceneInfo.iSceneID,dt); }
void CScene::Late_Update(float dt) { if(m_pGameManager) m_pGameManager->GameObjectManager_Late_Update(m_tSceneInfo.iSceneID,dt); }
void CScene::Add_RenderGroup() { if(m_pGameManager) m_pGameManager->GameObjectManager_Add_RenderGroup(m_tSceneInfo.iSceneID); }
HRESULT CScene::Render(){return S_OK;}
void CScene::OnEnter(){}
void CScene::OnExit(){}
void CScene::OnPause(){m_isPause=true;}
void CScene::OnResume(){m_isPause=false;}
void CScene::Free(){OnExit();if(m_pGameManager)m_pGameManager->Release();m_pGameManager=nullptr;m_pDevice=nullptr;m_pDeviceContext=nullptr;CBase::Free();}
