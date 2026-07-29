#pragma once

#include "CGameObject.h"

class CContainerObject;

class ENGINE_DLL CPartObject abstract : public CGameObject
{
public:
    struct PARTOBJECT_DESC : public Engine::GAMEOBJECT_DESC
    {
        const float* pParentWorldMatrix = nullptr;
        CContainerObject* pMasterContainerObject = nullptr;
    };

protected:
    explicit CPartObject(Device* pDevice, Context* pDeviceContext);
    explicit CPartObject(const CPartObject& Prototype);
    ~CPartObject() override = default;

public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    HRESULT Ready_Component(void* pArg) override;
    void Pre_Update(float fTimeDelta) override;
    void Update(float fTimeDelta) override;
    void Late_Update(float fTimeDelta) override;
    HRESULT Add_RenderGroup() override;
    HRESULT Render() override;

    CContainerObject* Get_MasterContainerObject() const { return m_pMasterContainerObject; }
    const float* Get_CombinedWorldMatrix() const { return m_matCombinedWorld; }
    void SetUp_CombinedWorldMatrix(const float* pLocalWorldMatrix);
    void Free() override;

protected:
    const float* m_pMasterWorldMatrix = nullptr;
    float m_matCombinedWorld[16]{};
    CContainerObject* m_pMasterContainerObject = nullptr;
};
