#pragma once

#include "CGameObject.h"

class CSpriteAnimator;
class CTransform;

class CTestObject final : public CGameObject
{
private:
    explicit CTestObject();
    explicit CTestObject(const CTestObject& Prototype);
    ~CTestObject() override = default;

public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    HRESULT Ready_Component(void* pArg) override;

    void Update(float fTimeDelta) override;
    HRESULT Add_RenderGroup() override;
    HRESULT Render() override;

    static CTestObject* Create();
    CGameObject* Clone(void* pArg) override;
    void Free() override;

private:
    HRESULT Ready_AnimationClips();

private:
    CSpriteAnimator* m_pSpriteAnimator = nullptr;
    CTransform* m_pTransform = nullptr;
    float m_fClipChangeTime = 0.f;
    uint32 m_iCurrentClipIndex = 0;
};
