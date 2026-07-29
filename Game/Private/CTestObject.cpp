#include "pch.h"
#include "CTestObject.h"

#include "CGameManager.h"
#include "CCameraObjectBase.h"
#include "CSpriteAnimator.h"
#include "CTexture.h"
#include "CTransform.h"
#include "CVIBuffer_Rect.h"

namespace
{
constexpr const wchar_t* TEST_CLIPS[] = {L"Idle", L"Walk", L"Attack"};
}

CTestObject::CTestObject()
    : CGameObject{nullptr, nullptr}
{
}

CTestObject::CTestObject(const CTestObject& Prototype)
    : CGameObject{Prototype}
{
}

HRESULT CTestObject::Proto_Initialize()
{
    return CGameObject::Proto_Initialize();
}

HRESULT CTestObject::Clone_Initialize(void* pArg)
{
    if (FAILED(CGameObject::Clone_Initialize(pArg)))
        return E_FAIL;
    return Ready_AnimationClips();
}

HRESULT CTestObject::Ready_Component(void*)
{
    Engine::TRANSFORM_DESC TransformDesc{};
    TransformDesc.vPosition[0] = 0.f;
    TransformDesc.vPosition[1] = 0.f;
    TransformDesc.vPosition[2] = 0.f;
    TransformDesc.vScale[0] = 3.f;
    TransformDesc.vScale[1] = 3.f;
    TransformDesc.vScale[2] = 1.f;
    if (FAILED(Add_Component(
            Engine::GLOBAL_LEVEL,
            L"Proto_Component_Transform",
            L"Com_Transform",
            &TransformDesc)))
        return E_FAIL;
    m_pTransform = static_cast<CTransform*>(Find_Component(L"Com_Transform"));
    if (m_pTransform == nullptr)
        return E_FAIL;

    CTexture* pTexture = static_cast<CTexture*>(m_pGameManager->ResourceManager_Get_Resource(
        L"Texture_Test_Skeleton", RESOURCE_TYPE::TEXTURE, false));
    if (pTexture == nullptr)
        return E_FAIL;

    CSpriteAnimator::SPRITEANIMATOR_DESC AnimatorDesc{};
    AnimatorDesc.pTexture = pTexture;
    AnimatorDesc.eTextureMode = SPRITE_TEXTURE_MODE::ATLAS_GRID;
    // 원본은 512x512이며 한 셀이 64x64라 실제 UV 분할은 8행 x 8열이다.
    // 그중 필요한 행만 Animation Clip으로 등록해서 사용한다.
    AnimatorDesc.iAtlasRowCount = 8;
    AnimatorDesc.iAtlasColumnCount = 8;
    AnimatorDesc.iTotalFrameCount = 8 * 8;
    AnimatorDesc.isAutoPlay = false;

    const HRESULT hr = Add_Component(
        Engine::GLOBAL_LEVEL,
        L"Proto_Component_SpriteAnimator",
        L"Com_SpriteAnimator",
        &AnimatorDesc);
    pTexture->Release();
    if (FAILED(hr))
        return E_FAIL;

    m_pSpriteAnimator = static_cast<CSpriteAnimator*>(Find_Component(L"Com_SpriteAnimator"));
    return m_pSpriteAnimator != nullptr ? S_OK : E_FAIL;
}

HRESULT CTestObject::Ready_AnimationClips()
{
    if (m_pSpriteAnimator == nullptr)
        return E_FAIL;

    // 행과 열은 0부터 시작한다. 실제 프레임 수만 등록하므로 오른쪽 빈 셀은 재생하지 않는다.
    const CSpriteAnimator::SPRITE_CLIP_DESC Idle{0, 0, 2, 3.f, true};
    const CSpriteAnimator::SPRITE_CLIP_DESC Walk{1, 0, 4, 7.f, true};
    const CSpriteAnimator::SPRITE_CLIP_DESC Attack{2, 0, 3, 8.f, true};

    if (FAILED(m_pSpriteAnimator->Add_Clip(L"Idle", Idle)) ||
        FAILED(m_pSpriteAnimator->Add_Clip(L"Walk", Walk)) ||
        FAILED(m_pSpriteAnimator->Add_Clip(L"Attack", Attack)))
        return E_FAIL;

    m_iCurrentClipIndex = 0;
    return m_pSpriteAnimator->Play_Clip(TEST_CLIPS[m_iCurrentClipIndex]);
}

void CTestObject::Update(float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    if (m_pSpriteAnimator == nullptr)
        return;

    m_pSpriteAnimator->Update(fTimeDelta);
    m_fClipChangeTime += fTimeDelta;
    if (m_fClipChangeTime >= 2.f)
    {
        m_fClipChangeTime = 0.f;
        m_iCurrentClipIndex = (m_iCurrentClipIndex + 1) % static_cast<uint32>(std::size(TEST_CLIPS));
        m_pSpriteAnimator->Play_Clip(TEST_CLIPS[m_iCurrentClipIndex]);
    }
}

HRESULT CTestObject::Add_RenderGroup()
{
    return m_pGameManager != nullptr
        ? m_pGameManager->RenderManager_Add_RenderGroup(RENDER_GROUP::BLEND, this)
        : E_FAIL;
}

HRESULT CTestObject::Render()
{
    if (m_pSpriteAnimator == nullptr || m_pTransform == nullptr || m_pGameManager == nullptr)
        return E_FAIL;

    CCameraObjectBase* pMainCamera = m_pGameManager->CameraManager_Get_MainCamera();
    CVIBuffer_Rect* pVIBuffer = m_pSpriteAnimator->Get_VIBuffer();
    if (pMainCamera == nullptr || pVIBuffer == nullptr)
        return E_FAIL;

    const Diligent::float4x4 WorldViewProjection =
        m_pTransform->Get_WorldMatrix() *
        pMainCamera->Get_ViewMatrix() *
        pMainCamera->Get_ProjectionMatrix();
    pVIBuffer->Set_WorldViewProjection(WorldViewProjection);
    return m_pSpriteAnimator->Render();
}

CTestObject* CTestObject::Create()
{
    CTestObject* pInstance = new CTestObject{};
    if (FAILED(pInstance->Proto_Initialize()))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CGameObject* CTestObject::Clone(void* pArg)
{
    CTestObject* pInstance = new CTestObject{*this};
    if (FAILED(pInstance->Clone_Initialize(pArg)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

void CTestObject::Free()
{
    m_pSpriteAnimator = nullptr;
    m_pTransform = nullptr;
    CGameObject::Free();
}
