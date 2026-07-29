#pragma once

#include "CBase.h"
#include "Engine_Enum.h"
#include "Engine_Struct.h"

class CBone;
class CChannel;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CAnimation final : public CBase
{
private:
    CAnimation() = default;
    explicit CAnimation(const CAnimation& Prototype);
    ~CAnimation() override = default;

public:
    HRESULT Initialize(const Engine::ANIMATION_INFO& tInfo);
    void Set_IsAnimationEnd(bool isEnd) { m_isAnimationEnd = isEnd; }
    void SetPlayType(ANIMATION_PLAYTYPE ePlayType) { m_ePlayType = ePlayType; }
    void ResetAnimation();
    bool Get_IsAnimationEnd() const { return m_isAnimationEnd; }
    float Get_AnimProgress() const;
    const std::string& Get_AnimationName() const { return m_strName; }
    void Update_TransformMatrix(float fSpeed, float fTimeDelta, const std::vector<CBone*>& vecBone, bool& isEnd);

    static CAnimation* Create(const Engine::ANIMATION_INFO& tInfo);
    CAnimation* Clone();
    void Free() override;

private:
    std::string m_strName;
    std::vector<CChannel*> m_vecChannel;
    std::vector<uint32> m_vecCurrentKeyFrameIndex;
    ANIMATION_PLAYTYPE m_ePlayType = ANIMATION_PLAYTYPE::LOOP;
    float m_fTicksPerSecond = 1.f;
    float m_fDuration = 0.f;
    float m_fCurrentTrackPosition = 0.f;
    bool m_isAnimationEnd = false;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
