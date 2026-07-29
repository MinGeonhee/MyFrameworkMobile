#pragma once

#include "CBase.h"
#include "Engine_Struct.h"

class CBone;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CChannel final : public CBase
{
private:
    CChannel() = default;
    explicit CChannel(const CChannel& Prototype);
    ~CChannel() override = default;

public:
    HRESULT Initialize(const Engine::CHANNEL_INFO& tInfo);
    uint32 Get_FrameCount() const { return static_cast<uint32>(m_vecKeyFrame.size()); }
    uint32 Get_ConnectedBoneIndex() const { return m_iConnectedBoneIndex; }
    Engine::KEY_FRAME Make_CurTrackPositionFrame(float fTrackPosition, uint32& iCurrentFrameIndex) const;
    void Update_ConnectedBoneTransformMatrix(float fTrackPosition,
                                             const std::vector<CBone*>& vecBone,
                                             uint32& iCurrentFrameIndex) const;

    static CChannel* Create(const Engine::CHANNEL_INFO& tInfo);
    CChannel* Clone();
    void Free() override;

private:
    uint32 m_iConnectedBoneIndex = 0;
    std::vector<Engine::KEY_FRAME> m_vecKeyFrame;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
