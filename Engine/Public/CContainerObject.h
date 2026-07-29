#pragma once

#include "CGameObject.h"

class CPartObject;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CContainerObject abstract : public CGameObject
{
public:
    struct CONTAINEROBJECT_DESC : public Engine::GAMEOBJECT_DESC {};

protected:
    explicit CContainerObject(Device* pDevice, Context* pDeviceContext);
    explicit CContainerObject(const CContainerObject& Prototype);
    ~CContainerObject() override = default;

public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    HRESULT Ready_Component(void* pArg) override;
    virtual HRESULT Ready_PartObject();
    void Pre_Update(float fTimeDelta) override;
    void Update(float fTimeDelta) override;
    void Late_Update(float fTimeDelta) override;
    HRESULT Add_RenderGroup() override;
    HRESULT Render() override;
    void Set_Dead() override;

    HRESULT Add_PartObject(uint32 iFindLevelID,
                           const std::wstring& strPrototypeTag,
                           const std::wstring& strPartTag,
                           void* pArg,
                           CPartObject** ppPartObject = nullptr);
    CPartObject* Find_PartObject(const std::wstring& strPartTag) const;
    const std::map<std::wstring, CPartObject*>& Get_PartObjectList() const { return m_mapPartObject; }
    void Free() override;

protected:
    std::map<std::wstring, CPartObject*> m_mapPartObject;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
