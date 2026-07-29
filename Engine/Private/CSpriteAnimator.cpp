#include "pch.h"
#include "CSpriteAnimator.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CTexture.h"
#include "CVIBuffer_Rect.h"

CSpriteAnimator::CSpriteAnimator(Device* pDevice, Context* pDeviceContext)
    : CComponent{pDevice, pDeviceContext} {}
CSpriteAnimator::CSpriteAnimator(const CSpriteAnimator& Prototype)
    : CComponent{Prototype} {}

HRESULT CSpriteAnimator::Proto_Initialize() { return CComponent::Proto_Initialize(); }

HRESULT CSpriteAnimator::Clone_Initialize(void* pArg)
{
    if (FAILED(CComponent::Clone_Initialize(pArg)) || pArg == nullptr) return E_INVALIDARG;
    const SPRITEANIMATOR_DESC& Desc = *static_cast<SPRITEANIMATOR_DESC*>(pArg);
    if (Desc.pTexture == nullptr || Desc.fFramePerSecond <= 0.f) return E_INVALIDARG;

    m_pTexture = Desc.pTexture; m_pTexture->AddRef();
    m_pShader = Desc.pShader;
    if (m_pShader != nullptr) m_pShader->AddRef();
    else m_pShader = static_cast<CShader*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(
        L"Shader_VIBuffer_Rect", RESOURCE_TYPE::SHADER, false));

    m_pVIBuffer = Desc.pVIBuffer;
    if (m_pVIBuffer != nullptr) m_pVIBuffer->AddRef();
    else m_pVIBuffer = static_cast<CVIBuffer_Rect*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype(
        PROTOTYPE_TYPE::COMPONENT, Engine::GLOBAL_LEVEL, L"Proto_Component_VIBuffer_Rect"));
    if (m_pShader == nullptr || m_pVIBuffer == nullptr) return E_FAIL;

    m_eTextureMode = Desc.eTextureMode;
    m_iAtlasColumnCount = std::max(Desc.iAtlasColumnCount, 1u);
    m_iAtlasRowCount = std::max(Desc.iAtlasRowCount, 1u);
    const uint32 iAvailableFrames = m_eTextureMode == SPRITE_TEXTURE_MODE::ATLAS_GRID
        ? m_iAtlasColumnCount * m_iAtlasRowCount : m_pTexture->Get_TextureCount();
    m_iTotalFrameCount = Desc.iTotalFrameCount == 0 ? iAvailableFrames : std::min(Desc.iTotalFrameCount, iAvailableFrames);
    if (m_iTotalFrameCount == 0) return E_INVALIDARG;
    m_iStartFrame = std::min(Desc.iStartFrame, m_iTotalFrameCount - 1);
    m_iEndFrame = Desc.iEndFrame == UINT32_MAX ? m_iTotalFrameCount - 1 : std::min(Desc.iEndFrame, m_iTotalFrameCount - 1);
    if (m_iStartFrame > m_iEndFrame) return E_INVALIDARG;
    m_iCurrentFrame = m_iStartFrame;
    m_fFramePerSecond = Desc.fFramePerSecond;
    m_isLoop = Desc.isLoop;
    m_isPlaying = Desc.isAutoPlay;
    m_pVIBuffer->Set_Shader(m_pShader);
    return Apply_CurrentFrame();
}

void CSpriteAnimator::Update(float fTimeDelta)
{
    if (!m_isPlaying || m_isPaused || fTimeDelta <= 0.f) return;
    const float fFrameTime = 1.f / m_fFramePerSecond;
    m_fAccTime += fTimeDelta;
    while (m_fAccTime >= fFrameTime && m_isPlaying)
    {
        m_fAccTime -= fFrameTime;
        Advance_Frame();
    }
}

void CSpriteAnimator::Advance_Frame()
{
    if (m_iCurrentFrame < m_iEndFrame) { ++m_iCurrentFrame; return; }
    if (m_isLoop) { m_iCurrentFrame = m_iStartFrame; return; }
    m_isPlaying = false; m_isPaused = false; m_iCurrentFrame = m_iEndFrame;
    if (m_PlayFinishedCallback) m_PlayFinishedCallback();
}

HRESULT CSpriteAnimator::Apply_CurrentFrame()
{
    if (m_pTexture == nullptr || m_pVIBuffer == nullptr) return E_FAIL;
    if (m_eTextureMode == SPRITE_TEXTURE_MODE::TEXTURE_SEQUENCE)
    {
        m_pVIBuffer->Set_Texture(m_pTexture, m_iCurrentFrame);
        m_pVIBuffer->Set_UVRect(0.f, 0.f, 1.f, 1.f);
    }
    else
    {
        const uint32 iColumn = m_iCurrentFrame % m_iAtlasColumnCount;
        const uint32 iRow = m_iCurrentFrame / m_iAtlasColumnCount;
        const float fScaleU = 1.f / static_cast<float>(m_iAtlasColumnCount);
        const float fScaleV = 1.f / static_cast<float>(m_iAtlasRowCount);
        m_pVIBuffer->Set_Texture(m_pTexture, 0);
        m_pVIBuffer->Set_UVRect(iColumn * fScaleU, iRow * fScaleV, fScaleU, fScaleV);
    }
    return S_OK;
}

HRESULT CSpriteAnimator::Render()
{
    if (FAILED(Apply_CurrentFrame())) return E_FAIL;
    return m_pVIBuffer->Render();
}

void CSpriteAnimator::Play(bool isRestart)
{ if (isRestart) { m_iCurrentFrame=m_iStartFrame; m_fAccTime=0.f; } m_isPlaying=true; m_isPaused=false; }
HRESULT CSpriteAnimator::Play(uint32 iStart,uint32 iEnd,bool isLoop)
{ if(iStart>iEnd||iEnd>=m_iTotalFrameCount)return E_INVALIDARG;m_strCurrentClipKey.clear();m_iStartFrame=iStart;m_iEndFrame=iEnd;m_isLoop=isLoop;Play(true);return S_OK; }

HRESULT CSpriteAnimator::Add_Clip(const std::wstring& strClipKey, const SPRITE_CLIP_DESC& Desc)
{
    if (m_eTextureMode != SPRITE_TEXTURE_MODE::ATLAS_GRID || strClipKey.empty() ||
        Desc.iRowIndex >= m_iAtlasRowCount || Desc.iFrameCount == 0 ||
        Desc.fFramePerSecond <= 0.f ||
        Desc.iStartColumn >= m_iAtlasColumnCount ||
        Desc.iStartColumn + Desc.iFrameCount > m_iAtlasColumnCount)
        return E_INVALIDARG;

    if (!m_Clips.emplace(strClipKey, Desc).second)
        return E_FAIL;
    return S_OK;
}

HRESULT CSpriteAnimator::Play_Clip(const std::wstring& strClipKey, bool isRestart)
{
    const auto Iter = m_Clips.find(strClipKey);
    if (Iter == m_Clips.end())
        return E_FAIL;

    const SPRITE_CLIP_DESC& Desc = Iter->second;
    const uint32 iStartFrame = Desc.iRowIndex * m_iAtlasColumnCount + Desc.iStartColumn;
    const uint32 iEndFrame = iStartFrame + Desc.iFrameCount - 1;
    if (iEndFrame >= m_iTotalFrameCount)
        return E_FAIL;

    const bool isSameClip = m_strCurrentClipKey == strClipKey;
    m_strCurrentClipKey = strClipKey;
    m_iStartFrame = iStartFrame;
    m_iEndFrame = iEndFrame;
    m_iCurrentFrame = (!isSameClip || isRestart) ? iStartFrame : m_iCurrentFrame;
    m_fFramePerSecond = Desc.fFramePerSecond;
    m_isLoop = Desc.isLoop;
    m_fAccTime = (!isSameClip || isRestart) ? 0.f : m_fAccTime;
    m_isPlaying = true;
    m_isPaused = false;
    return Apply_CurrentFrame();
}

bool CSpriteAnimator::Has_Clip(const std::wstring& strClipKey) const
{
    return m_Clips.find(strClipKey) != m_Clips.end();
}
void CSpriteAnimator::Pause(){if(m_isPlaying)m_isPaused=true;}
void CSpriteAnimator::Resume(){if(m_isPlaying)m_isPaused=false;}
void CSpriteAnimator::Stop(bool reset){m_isPlaying=false;m_isPaused=false;m_fAccTime=0.f;if(reset)m_iCurrentFrame=m_iStartFrame;}
HRESULT CSpriteAnimator::Set_CurrentFrame(uint32 iFrame){if(iFrame>=m_iTotalFrameCount)return E_INVALIDARG;m_iCurrentFrame=iFrame;m_fAccTime=0.f;return Apply_CurrentFrame();}
void CSpriteAnimator::Set_FramePerSecond(float fps){if(fps>0.f)m_fFramePerSecond=fps;}

CSpriteAnimator* CSpriteAnimator::Create(Device* pDevice, Context* pDeviceContext)
{ auto* p=new CSpriteAnimator{pDevice,pDeviceContext};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p; }
CComponent* CSpriteAnimator::Clone(void* pArg)
{ auto* p=new CSpriteAnimator{*this};if(FAILED(p->Clone_Initialize(pArg))){p->Release();return nullptr;}return p; }
void CSpriteAnimator::Free()
{
    m_PlayFinishedCallback={};
    m_Clips.clear();
    m_strCurrentClipKey.clear();
    if(m_pVIBuffer)m_pVIBuffer->Release();m_pVIBuffer=nullptr;
    if(m_pShader)m_pShader->Release();m_pShader=nullptr;
    if(m_pTexture)m_pTexture->Release();m_pTexture=nullptr;
    CComponent::Free();
}
