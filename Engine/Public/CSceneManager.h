#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
#include <functional>
#include <queue>
class CScene;
using SceneFactory = std::function<CScene*(const Engine::CREATE_SCENE_DESC&)>;
class ENGINE_DLL CSceneManager final : public CBase
{
private:
    CSceneManager()=default;
    HRESULT ChangeScene(const Engine::CREATE_SCENE_DESC& Desc);
    CScene* MakeSceneByFactory(const Engine::CREATE_SCENE_DESC& Desc) const;
public:
    ~CSceneManager() override=default;
    HRESULT Register_SceneFactory(uint32 iSceneID,const SceneFactory& Factory);
    void RequestChangeScene(const Engine::CREATE_SCENE_DESC& Desc);
    HRESULT ExecutePendingSceneChanges();
    uint32 GetCurSceneID() const;
    CScene* GetCurScene() const;
    const std::vector<CScene*>& GetStackScene() const{return m_stackScene;}
    void Pre_Update(float dt);void Update(float dt);void Late_Update(float dt);void Add_RenderGroup();HRESULT Render();
    static CSceneManager* Create();void Free() override;
private:
    std::vector<CScene*> m_stackScene;
    std::queue<Engine::CREATE_SCENE_DESC> m_queueSceneChange;
    std::unordered_map<uint32,SceneFactory> m_mapSceneFactory;
};
