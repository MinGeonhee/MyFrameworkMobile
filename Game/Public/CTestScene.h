#pragma once
#include "CScene.h"
class CLoader;
class CDefaultCameraObject;
class CTestScene final:public CScene
{
private:CTestScene();~CTestScene()override=default;
public:
    HRESULT Initialize(const Engine::CREATE_SCENE_DESC& CreateDesc,const Engine::SCENE_INFO& SceneInfo)override;
    void Update(float fTimeDelta)override;
    void OnExit()override;
    static CTestScene* Create(const Engine::CREATE_SCENE_DESC& CreateDesc);
    void Free()override;
private:
    HRESULT Ready_DefaultCamera();
    HRESULT Ready_TestObject();
    HRESULT Ready_World();
    CLoader*m_pLoader=nullptr;
};
