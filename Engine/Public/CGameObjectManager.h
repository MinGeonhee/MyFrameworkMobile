#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CLayer;
class CGameObject;
class CGameManager;

class CGameObjectManager :
    public CBase
{
private:
    explicit                    CGameObjectManager( ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual                     ~CGameObjectManager() {}
private:
    HRESULT                     Initialize(CONST GameObjectManager_Desc& tDesc);
private:
    CLayer*                     Find_Layer(uint32 iFindSceneID, const wstring& strFindLayerKey);
    CLayer*                     Find_Layer_InGlobal(const wstring& strFindLayerKey);
public:
    CGameObject*                Find_GameObject_InGlobal(const wstring& strFindLayerKey, const wstring& strGameObjectName);
    CGameObject*                Find_GameObject_InGlobal(const wstring& strFindLayerKey, CGameObject* pFindGameObject);
    CGameObject*                Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, CGameObject* pFindGameObject);
    CGameObject*                Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, const wstring& strGameObjectName);
    CGameObject*                Add_GameObject(uint32 iFindProtoSceneID, const wstring& strFindLayerkey, uint32 iAddCloneSceneID, const wstring& strAddLayerKey, void* pArg);
    CGameObject*                Add_GameObject_ByGlobalProto(const wstring& strFindProtoKey, const wstring& strAddLayerKey,void* pArg = nullptr, uint32 iAddCloneSceneID = GLOBAL);
public:
    void                        Pre_Update( uint32 iSceneID , float DT);
    void                        Update(uint32 iSceneID,float DT);
    void                        Late_Update(uint32 iSceneID,float DT);
    HRESULT                     Add_RenderGroup(uint32 iSceneID);
    void                        Clean_DeadObject();

public:
    void                        Add_ColliderGroup(uint32 iSceneID);
public:
    const map<const wstring, CLayer*>&          GetCurSceneLayer(uint32 iCurSceneID) { return m_mapLayer[iCurSceneID]; }
    const vector<map<const wstring, CLayer*>>&  Get_AllSceneLayer() { return m_mapLayer; }
    const map<const wstring, CLayer*>&          GetGlobalLayer() { return m_mapGlobalLayers; }
    const CLayer*                               Get_Layer(uint32 iSceneID, const wstring& wstrLayerName);
public:
    HRESULT                                     Clear_GameObjectByScneeID(uint32 iSceneID);
private:
    uint32                                      m_iTotalSceneNum;
    vector<map<const wstring, CLayer*>>         m_mapLayer;
    map<const wstring, CLayer*>                 m_mapGlobalLayers;
private:
    ComPtr<Device>                              m_pDevice;
    ComPtr<DeviceContext>                       m_pDeviceContext;
    CGameManager*                               m_pGameManager;
public:
    static CGameObjectManager* Create(CONST GameObjectManager_Desc& tDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual void    Free() override;
};

NS_END

