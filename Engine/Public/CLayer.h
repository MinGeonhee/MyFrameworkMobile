#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CGameObject;

class CLayer final : public CBase
{
private:
    explicit CLayer();
    virtual ~CLayer() {};
private:
    HRESULT Initialize();
public:
    HRESULT         Add_GameObject(CGameObject* pGameObject);
    CGameObject*    Find_GameObject(CGameObject* pGameObject);
    CGameObject*    Find_GameObject(const wstring& strGameObjectName);
public:
    void            Pre_Update(float DT);
    void            Update(float DT);
    void            Late_Update(float DT);
    HRESULT         Add_RenderGroup();
    void            Render();
    HRESULT         ClearGameObject();
    void            Clean_DeadObject();
    void            Add_ColliderGroup();
public:
    const   vector<CGameObject*>& GetGameObjectalist() const { return m_vecGameObject; }
    bool            IsEmpty()                           { return m_vecGameObject.empty();}
    wstring         GetGameObjectName(uint32 iIndex);
private:
    /* vector로 한번 더 감싸야한다... */
    vector<CGameObject*> m_vecGameObject;


public:
    static  CLayer* Create();
    virtual void    Free() override;
};

NS_END