#include "pch.h"
#include "CAnimation.h"
#include "CBone.h"
#include "CChannel.h"

CAnimation::CAnimation(const CAnimation& Prototype)
    : CBase{Prototype}
    , m_strName{Prototype.m_strName}
    , m_ePlayType{Prototype.m_ePlayType}
    , m_fTicksPerSecond{Prototype.m_fTicksPerSecond}
    , m_fDuration{Prototype.m_fDuration}
{
    for (CChannel* pChannel : Prototype.m_vecChannel)
        m_vecChannel.push_back(pChannel != nullptr ? pChannel->Clone() : nullptr);
    m_vecCurrentKeyFrameIndex.resize(m_vecChannel.size(), 0);
}

HRESULT CAnimation::Initialize(const Engine::ANIMATION_INFO& tInfo)
{
    if (tInfo.strName.empty() || tInfo.fDuration <= 0.f)
        return E_INVALIDARG;
    m_strName = tInfo.strName;
    m_fDuration = tInfo.fDuration;
    m_fTicksPerSecond = tInfo.fTicksPerSecond > 0.f ? tInfo.fTicksPerSecond : 1.f;
    for (const auto& ChannelInfo : tInfo.vecChannel)
    {
        CChannel* pChannel = CChannel::Create(ChannelInfo);
        if (pChannel == nullptr)
            return E_FAIL;
        m_vecChannel.push_back(pChannel);
    }
    m_vecCurrentKeyFrameIndex.resize(m_vecChannel.size(), 0);
    return S_OK;
}

void CAnimation::ResetAnimation()
{
    m_fCurrentTrackPosition = 0.f;
    m_isAnimationEnd = false;
    std::fill(m_vecCurrentKeyFrameIndex.begin(), m_vecCurrentKeyFrameIndex.end(), 0);
}

float CAnimation::Get_AnimProgress() const
{
    return m_fDuration > 0.f ? std::clamp(m_fCurrentTrackPosition / m_fDuration, 0.f, 1.f) : 0.f;
}

void CAnimation::Update_TransformMatrix(float fSpeed,
                                        float fTimeDelta,
                                        const std::vector<CBone*>& vecBone,
                                        bool& isEnd)
{
    if (m_isAnimationEnd && m_ePlayType != ANIMATION_PLAYTYPE::LOOP)
    {
        isEnd = true;
        return;
    }
    m_fCurrentTrackPosition += m_fTicksPerSecond * fSpeed * fTimeDelta;
    if (m_fCurrentTrackPosition >= m_fDuration)
    {
        if (m_ePlayType == ANIMATION_PLAYTYPE::LOOP)
            ResetAnimation();
        else
        {
            m_fCurrentTrackPosition = m_fDuration;
            m_isAnimationEnd = true;
        }
    }
    for (size_t i = 0; i < m_vecChannel.size(); ++i)
        if (m_vecChannel[i] != nullptr)
            m_vecChannel[i]->Update_ConnectedBoneTransformMatrix(
                m_fCurrentTrackPosition, vecBone, m_vecCurrentKeyFrameIndex[i]);
    isEnd = m_isAnimationEnd;
}

CAnimation* CAnimation::Create(const Engine::ANIMATION_INFO& tInfo)
{
    CAnimation* pInstance = new CAnimation{};
    if (FAILED(pInstance->Initialize(tInfo))) { pInstance->Release(); return nullptr; }
    return pInstance;
}
CAnimation* CAnimation::Clone() { return new CAnimation{*this}; }

void CAnimation::Free()
{
    for (CChannel* pChannel : m_vecChannel)
        if (pChannel != nullptr)
            pChannel->Release();
    m_vecChannel.clear();
    m_vecCurrentKeyFrameIndex.clear();
    CBase::Free();
}
