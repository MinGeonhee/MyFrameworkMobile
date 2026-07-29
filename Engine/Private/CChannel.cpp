#include "pch.h"
#include "CChannel.h"
#include "CBone.h"
#include <cmath>

namespace
{
float Lerp(float a, float b, float t) { return a + (b - a) * t; }

void MakeSRT(const Engine::KEY_FRAME& Frame, float* m)
{
    float x = Frame.vRotation[0], y = Frame.vRotation[1], z = Frame.vRotation[2], w = Frame.vRotation[3];
    const float Length = std::sqrt(x*x + y*y + z*z + w*w);
    if (Length > 0.f) { x /= Length; y /= Length; z /= Length; w /= Length; }
    m[0]=(1-2*y*y-2*z*z)*Frame.vScale[0]; m[1]=(2*x*y+2*w*z)*Frame.vScale[0]; m[2]=(2*x*z-2*w*y)*Frame.vScale[0]; m[3]=0;
    m[4]=(2*x*y-2*w*z)*Frame.vScale[1]; m[5]=(1-2*x*x-2*z*z)*Frame.vScale[1]; m[6]=(2*y*z+2*w*x)*Frame.vScale[1]; m[7]=0;
    m[8]=(2*x*z+2*w*y)*Frame.vScale[2]; m[9]=(2*y*z-2*w*x)*Frame.vScale[2]; m[10]=(1-2*x*x-2*y*y)*Frame.vScale[2]; m[11]=0;
    m[12]=Frame.vTranslation[0]; m[13]=Frame.vTranslation[1]; m[14]=Frame.vTranslation[2]; m[15]=1;
}
}

CChannel::CChannel(const CChannel& Prototype)
    : CBase{Prototype}, m_iConnectedBoneIndex{Prototype.m_iConnectedBoneIndex}, m_vecKeyFrame{Prototype.m_vecKeyFrame}
{
}

HRESULT CChannel::Initialize(const Engine::CHANNEL_INFO& tInfo)
{
    if (tInfo.vecKeyFrame.empty())
        return E_INVALIDARG;
    m_iConnectedBoneIndex = tInfo.iConnectedBoneIndex;
    m_vecKeyFrame = tInfo.vecKeyFrame;
    return S_OK;
}

Engine::KEY_FRAME CChannel::Make_CurTrackPositionFrame(float fTrackPosition, uint32& iCurrentFrameIndex) const
{
    if (m_vecKeyFrame.size() == 1)
        return m_vecKeyFrame.front();
    while (iCurrentFrameIndex + 1 < m_vecKeyFrame.size() &&
           fTrackPosition >= m_vecKeyFrame[iCurrentFrameIndex + 1].fTime)
        ++iCurrentFrameIndex;
    const uint32 iNext = std::min<uint32>(iCurrentFrameIndex + 1, static_cast<uint32>(m_vecKeyFrame.size() - 1));
    const auto& A = m_vecKeyFrame[iCurrentFrameIndex];
    const auto& B = m_vecKeyFrame[iNext];
    const float Range = B.fTime - A.fTime;
    const float t = Range > 0.f ? std::clamp((fTrackPosition - A.fTime) / Range, 0.f, 1.f) : 0.f;
    Engine::KEY_FRAME Result{};
    Result.fTime = fTrackPosition;
    for (uint32 i=0;i<3;++i) { Result.vScale[i]=Lerp(A.vScale[i],B.vScale[i],t); Result.vTranslation[i]=Lerp(A.vTranslation[i],B.vTranslation[i],t); }
    for (uint32 i=0;i<4;++i) Result.vRotation[i]=Lerp(A.vRotation[i],B.vRotation[i],t);
    return Result;
}

void CChannel::Update_ConnectedBoneTransformMatrix(float fTrackPosition,
                                                   const std::vector<CBone*>& vecBone,
                                                   uint32& iCurrentFrameIndex) const
{
    if (m_iConnectedBoneIndex >= vecBone.size() || vecBone[m_iConnectedBoneIndex] == nullptr)
        return;
    float Matrix[16]{};
    MakeSRT(Make_CurTrackPositionFrame(fTrackPosition, iCurrentFrameIndex), Matrix);
    vecBone[m_iConnectedBoneIndex]->Set_TransformMatrix(Matrix);
}

CChannel* CChannel::Create(const Engine::CHANNEL_INFO& tInfo)
{
    CChannel* pInstance = new CChannel{};
    if (FAILED(pInstance->Initialize(tInfo))) { pInstance->Release(); return nullptr; }
    return pInstance;
}
CChannel* CChannel::Clone() { return new CChannel{*this}; }
void CChannel::Free() { m_vecKeyFrame.clear(); CBase::Free(); }
