#pragma once
#include "CBase.h"
#include "Engine_Struct.h"

class CComponent;
class CGameManager;
class CCollider;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
    explicit CGameObject(Device* pDevice, Context* pDeviceContext);
    explicit CGameObject(const CGameObject& Prototype);
    ~CGameObject() override = default;

public:
    virtual HRESULT Proto_Initialize();
    virtual HRESULT Clone_Initialize(void* pArg);
    virtual HRESULT Ready_Component(void* pArg);

    HRESULT Add_Component(uint32 iFindLevelID,
                          const std::wstring& strPrototypeTag,
                          const std::wstring& strComponentTag,
                          void* pArg,
                          CComponent** ppOut = nullptr);
    CComponent* Find_Component(const std::wstring& strComponentTag) const;

    virtual void Pre_Update(float fTimeDelta);
    virtual void Update(float fTimeDelta);
    virtual void Late_Update(float fTimeDelta);
    virtual HRESULT Render();
    virtual HRESULT Add_RenderGroup();
    virtual void Add_ColliderGroup();
    virtual void OnCollisionEnter(CGameObject* pOther, CCollider* pOtherCollider);
    virtual void OnCollision(CGameObject* pOther, CCollider* pOtherCollider);
    virtual void OnCollisionExit(CGameObject* pOther, CCollider* pOtherCollider);

    virtual void Set_Dead();
    bool Get_Dead() const { return m_isDead; }
    const std::wstring& GetName() const { return m_strName; }
    void SetName(const std::wstring& strName) { m_strName = strName; }
    bool Get_IsUpdatePhaseActive(UPDATE_PHASE ePhase) const;
    void Set_UpdatePhaseActive(UPDATE_PHASE ePhase, bool isActive);
    void Set_UpdatePhaseAllActive(bool isActive);
    MAKE_TYPE Get_MakeType() const { return m_eMakeType; }
    bool Get_IsPartObject() const { return m_isPartObject; }
    void Set_IsPartObject(bool isPartObject) { m_isPartObject = isPartObject; }

    const std::map<std::wstring, CComponent*>& Get_ComponentList() const { return m_mapComponent; }

protected:
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
    CGameManager* m_pGameManager = nullptr;
    std::map<std::wstring, CComponent*> m_mapComponent;
    std::wstring m_strName;
    std::array<bool, static_cast<size_t>(UPDATE_PHASE::END)> m_isUpdatePhaseActive{};
    bool m_isDead = false;
    bool m_isExpectedDelete = false;
    bool m_isPartObject = false;
    uint32 m_iIndex = 0;
    int32 m_iEventIndex = -1;
    MAKE_TYPE m_eMakeType = MAKE_TYPE::PROTOTYPE;

public:
    virtual CGameObject* Clone(void* pArg) = 0;
    void Free() override;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
