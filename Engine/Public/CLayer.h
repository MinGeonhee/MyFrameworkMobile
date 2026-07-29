#pragma once

#include "CBase.h"

class CGameObject;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CLayer final : public CBase
{
private:
    CLayer() = default;
    ~CLayer() override = default;
    HRESULT Initialize();

public:
    HRESULT Add_GameObject(CGameObject* pGameObject);
    CGameObject* Find_GameObject(CGameObject* pGameObject) const;
    CGameObject* Find_GameObject(const std::wstring& strGameObjectName) const;
    void Pre_Update(float fTimeDelta);
    void Update(float fTimeDelta);
    void Late_Update(float fTimeDelta);
    HRESULT Add_RenderGroup();
    void Add_ColliderGroup();
    void Clean_DeadObject();
    void Clear_GameObject();
    const std::vector<CGameObject*>& Get_GameObjectList() const { return m_vecGameObject; }
    bool IsEmpty() const { return m_vecGameObject.empty(); }

    static CLayer* Create();
    void Free() override;

private:
    std::vector<CGameObject*> m_vecGameObject;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
