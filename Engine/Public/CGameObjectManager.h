#pragma once

#include "CBase.h"

#include <map>
#include <string>
#include <vector>

class CGameObject;
class CLayer;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CGameObjectManager final : public CBase
{
private:
    explicit CGameObjectManager(Device* pDevice, Context* pDeviceContext);
    HRESULT Initialize(uint32 iTotalLevelCount);
    CLayer* Find_Layer(uint32 iLevelID, const std::wstring& strLayerTag) const;

public:
    ~CGameObjectManager() override = default;
    CGameObject* Find_GameObject(uint32 iLevelID,
                                 const std::wstring& strLayerTag,
                                 const std::wstring& strGameObjectName) const;
    CGameObject* Find_GameObject(uint32 iLevelID,
                                 const std::wstring& strLayerTag,
                                 CGameObject* pGameObject) const;
    CGameObject* Add_GameObject(uint32 iPrototypeLevelID,
                                const std::wstring& strPrototypeTag,
                                uint32 iCloneLevelID,
                                const std::wstring& strLayerTag,
                                void* pArg = nullptr);
    HRESULT Add_GameObject_ToLayer(uint32 iLevelID,
                                   const std::wstring& strLayerTag,
                                   CGameObject* pGameObject);

    void Pre_Update(uint32 iLevelID, float fTimeDelta);
    void Update(uint32 iLevelID, float fTimeDelta);
    void Late_Update(uint32 iLevelID, float fTimeDelta);
    HRESULT Add_RenderGroup(uint32 iLevelID);
    void Add_ColliderGroup(uint32 iLevelID);
    void Clean_DeadObject();
    HRESULT Clear_GameObjectByLevelID(uint32 iLevelID);

    const std::map<std::wstring, CLayer*>* Get_LevelLayers(uint32 iLevelID) const;
    CLayer* Get_Layer(uint32 iLevelID, const std::wstring& strLayerTag) const;

    static CGameObjectManager* Create(uint32 iTotalLevelCount,
                                      Device* pDevice,
                                      Context* pDeviceContext);
    void Free() override;

private:
    uint32 m_iTotalLevelCount = 0;
    std::vector<std::map<std::wstring, CLayer*>> m_LevelLayers;
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

