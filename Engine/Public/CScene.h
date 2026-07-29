#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
class CGameManager;
class ENGINE_DLL CScene abstract : public CBase
{
protected:
    explicit CScene(Device* pDevice, Context* pDeviceContext);
    ~CScene() override = default;
public:
    virtual HRESULT Initialize(const Engine::CREATE_SCENE_DESC& CreateDesc, const Engine::SCENE_INFO& SceneInfo);
    virtual void Pre_Update(float fTimeDelta);
    virtual void Update(float fTimeDelta);
    virtual void Late_Update(float fTimeDelta);
    virtual void Add_RenderGroup();
    virtual HRESULT Render();
    virtual void OnEnter();
    virtual void OnExit();
    virtual void OnPause();
    virtual void OnResume();
    uint32 GetSceneID() const { return m_tSceneInfo.iSceneID; }
    const Engine::SCENE_INFO& GetSceneInfo() const { return m_tSceneInfo; }
    bool Get_IsPause() const { return m_isPause; }
    void Set_IsPause(bool isPause) { m_isPause = isPause; }
    void Free() override;
protected:
    bool m_isPause = false;
    Engine::CREATE_SCENE_DESC m_tCreateSceneDesc{};
    Engine::SCENE_INFO m_tSceneInfo{};
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
    CGameManager* m_pGameManager = nullptr;
};
