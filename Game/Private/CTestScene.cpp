#include "pch.h"
#include "CTestScene.h"

#include "CDefaultCameraObject.h"
#include "CGameManager.h"
#include "CLoader.h"
#include "CTestObject.h"
#include "CTerrain.h"
#include "CSkyBox.h"

namespace
{
constexpr uint32 TEST_SCENE_ID = 1;
}

CTestScene::CTestScene()
    : CScene{nullptr, nullptr}
{
}

HRESULT CTestScene::Initialize(const Engine::CREATE_SCENE_DESC& CreateDesc,
                               const Engine::SCENE_INFO& SceneInfo)
{
    if (FAILED(CScene::Initialize(CreateDesc, SceneInfo)) ||
        FAILED(Ready_DefaultCamera()) ||
        FAILED(Ready_World()) ||
        FAILED(Ready_TestObject()))
        return E_FAIL;

    m_pLoader = CLoader::Create();
    if (m_pLoader == nullptr)
        return E_FAIL;
    if (FAILED(m_pLoader->Add_Task(L"TestScene CPU 데이터 준비", [] { return S_OK; })) ||
        FAILED(m_pLoader->Start()))
        return E_FAIL;
    return S_OK;
}

HRESULT CTestScene::Ready_World()
{
    if(FAILED(m_pGameManager->ResourceManager_Load_Texture(L"Texture_SkyBox_Dawn",L"../../../Resource/Test/skybox-dawn.png")))return E_FAIL;
    CTerrain* terrain=CTerrain::Create();CSkyBox* sky=CSkyBox::Create();
    if(!terrain||!sky){if(terrain)terrain->Release();if(sky)sky->Release();return E_FAIL;}
    if(FAILED(m_pGameManager->PrototypeManager_Add_Prototype(PROTOTYPE_TYPE::GAMEOBJECT,TEST_SCENE_ID,L"Proto_GameObject_Terrain",terrain))||FAILED(m_pGameManager->PrototypeManager_Add_Prototype(PROTOTYPE_TYPE::GAMEOBJECT,TEST_SCENE_ID,L"Proto_GameObject_SkyBox",sky)))return E_FAIL;
    CTerrain::TERRAIN_DESC td{};td.strGameObjectName=L"Terrain_10x10";td.iNumCellsX=10;td.iNumCellsZ=10;td.fCellSize=1.f;
    Engine::GAMEOBJECT_DESC sd{};sd.strGameObjectName=L"SkyBox_Dawn";
    if(!m_pGameManager->GameObjectManager_Add_GameObject(TEST_SCENE_ID,L"Proto_GameObject_Terrain",TEST_SCENE_ID,L"Layer_Terrain",&td))return E_FAIL;
    if(!m_pGameManager->GameObjectManager_Add_GameObject(TEST_SCENE_ID,L"Proto_GameObject_SkyBox",TEST_SCENE_ID,L"Layer_SkyBox",&sd))return E_FAIL;
    return S_OK;
}

HRESULT CTestScene::Ready_DefaultCamera()
{
    CDefaultCameraObject::DefaultCameraObject_Desc Desc{};
    Desc.strGameObjectName = L"Camera_Default";
    Desc.eCameraType = CAMERA_TYPE::FREE;
    Desc.eMoveType = MOVE_TYPE::NORMAL;
    Desc.eUpdateMode = UPDATE_MODE::DYNAMIC;
    Desc.eProjectionType = PROJECTION_TYPE::PERSPECTIVE;
    Desc.fFilmingSizeX = 1280.f;
    Desc.fFilmingSizeY = 720.f;
    Desc.fFovDegree = 60.f;
    Desc.fNear = .1f;
    Desc.fFar = 1000.f;
    // 자유시점 확인용 기본 위치: 위쪽에서 원점을 비스듬히 내려다본다.
    Desc.TransformDesc.vPosition[1] = 10.f;
    Desc.TransformDesc.vPosition[2] = -10.f;
    Desc.TransformDesc.vRotationRadian[0] = 0.78539816f;
    Desc.TransformDesc.fSpeedPerSecond = 8.f;

    CDefaultCameraObject* pCamera = static_cast<CDefaultCameraObject*>(
        m_pGameManager->PrototypeManager_Clone_Prototype(
            PROTOTYPE_TYPE::GAMEOBJECT,
            Engine::GLOBAL_LEVEL,
            L"Proto_GameObject_DefaultCamera",
            &Desc));
    if (pCamera == nullptr)
        return E_FAIL;

    const HRESULT hr = m_pGameManager->CameraManager_Add_Camera(
        L"Camera_Default", Desc.eUpdateMode, pCamera);
    pCamera->Release();
    if (FAILED(hr))
        return E_FAIL;
    return m_pGameManager->CameraManager_Switch_MainCamera(L"Camera_Default", Desc.eUpdateMode);
}

HRESULT CTestScene::Ready_TestObject()
{
    // 실행 폴더는 Client/Bin/Debug 또는 Release이므로 프로젝트 루트까지 세 단계 올라간다.
    if (FAILED(m_pGameManager->ResourceManager_Load_Texture(
            L"Texture_Test_Skeleton",
            L"../../../Resource/Test/Skeleton.png")))
        return E_FAIL;

    CTestObject* pPrototype = CTestObject::Create();
    if (pPrototype == nullptr)
        return E_FAIL;
    const HRESULT hr = m_pGameManager->PrototypeManager_Add_Prototype(
        PROTOTYPE_TYPE::GAMEOBJECT,
        TEST_SCENE_ID,
        L"Proto_GameObject_TestObject",
        pPrototype);
    if (FAILED(hr))
    {
        pPrototype->Release();
        return E_FAIL;
    }

    Engine::GAMEOBJECT_DESC ObjectDesc{};
    ObjectDesc.strGameObjectName = L"TestObject_Skeleton";
    return m_pGameManager->GameObjectManager_Add_GameObject(
        TEST_SCENE_ID,
        L"Proto_GameObject_TestObject",
        TEST_SCENE_ID,
        L"Layer_TestObject",
        &ObjectDesc) != nullptr ? S_OK : E_FAIL;
}

void CTestScene::Update(float fTimeDelta)
{
    CScene::Update(fTimeDelta);
}

void CTestScene::OnExit()
{
    if (m_pGameManager == nullptr)
        return;
    m_pGameManager->CameraManager_Remove_Camera(L"Camera_Default", UPDATE_MODE::DYNAMIC);
    m_pGameManager->GameObjectManager_Clear(TEST_SCENE_ID);
    m_pGameManager->PrototypeManager_Clear_Prototype(TEST_SCENE_ID);
}

CTestScene* CTestScene::Create(const Engine::CREATE_SCENE_DESC& CreateDesc)
{
    CTestScene* pScene = new CTestScene{};
    Engine::SCENE_INFO SceneInfo{};
    SceneInfo.iSceneID = TEST_SCENE_ID;
    SceneInfo.eStackType = SCENE_STACK_TYPE::REPLACE;
    if (FAILED(pScene->Initialize(CreateDesc, SceneInfo)))
    {
        pScene->Release();
        return nullptr;
    }
    return pScene;
}

void CTestScene::Free()
{
    if (m_pLoader != nullptr)
        m_pLoader->Release();
    m_pLoader = nullptr;
    CScene::Free();
}
