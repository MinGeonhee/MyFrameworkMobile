#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CLayer;

class CPrototypeManager final: public CBase
{
private:
    explicit CPrototypeManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual ~CPrototypeManager() {}
private:
    HRESULT                 Initialize(CONST PrototypeManager_Desc& tDesc);

public:
    HRESULT                 Ready_DefaultPrototype(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);

private:
    CBase*                  Find_Prototype(PROTOTYPE_TYPE eProtoType, uint32 iProtoSceneID, const wstring& strProtoKey);
    CBase*                  Find_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType,const wstring& strProtoKey);
public:
    HRESULT                 Add_Prototype( PROTOTYPE_TYPE eAddProtoType , uint32 iAddProtoSceneID , const wstring& strAddProtoKey , CBase* pPrototype);
    HRESULT                 Add_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType, const wstring& strAddProtoKey, CBase* pPrototype);
    CBase*                  Clone_Prototype(PROTOTYPE_TYPE eCloneProtoType, uint32 iCloneProtoSceneID, const wstring& strCloneProtoKey, void* pArg);
    CBase*                  Clone_Prototype_InGlobal(PROTOTYPE_TYPE eCloneProtoType,const wstring& strCloneProtoKey, void* pArg);
public:
    void                    Clear_Prototype_CurSceneID(uint32 iCurSceneID);
public:

    map<const wstring, CBase*>&                 GetGameObjectPrototypeList_ByScene(uint32 iSceneID);
    map<const wstring, CBase*>&                 GetComponentPrototypeList_ByScene(uint32 iSceneID);
    const map<const wstring, CBase*>&           GetGlobalGameObjectPrototypeList() { return m_mapPrototypeGlobal[(uint32)PROTOTYPE_TYPE::GAMEOBJECT];}
    const map<const wstring, CBase*>&           GetGlobalComponentPrototypeList() { return m_mapPrototypeGlobal[(uint32)PROTOTYPE_TYPE::COMPONENT];}

private:
    uint32                                      m_iTotalSceneNum;
    vector<map<const wstring, CBase*>>          m_mapPrototype[Cast_Int(PROTOTYPE_TYPE::END)];
    map<const wstring, CBase*>                  m_mapPrototypeGlobal[Cast_Int(PROTOTYPE_TYPE::END)];
private:
    ComPtr<Device>                              m_pDevice;
    ComPtr<DeviceContext>                       m_pDeviceContext;
public:
    static CPrototypeManager* Create( CONST PrototypeManager_Desc& tDesc ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual void Free()override;
};

NS_END

