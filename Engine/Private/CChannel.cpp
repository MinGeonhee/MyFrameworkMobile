#include "CChannel.h"
#include "CBone.h"



CChannel::CChannel()
    : m_vecKeyFrame{}, m_iConnectedBoneIndex{}
{
}

CChannel::CChannel(const CChannel& Proto)
    : m_vecKeyFrame(Proto.m_vecKeyFrame), m_iConnectedBoneIndex(Proto.m_iConnectedBoneIndex)
{
}

CChannel::~CChannel()
{
}

HRESULT CChannel::Initialize(const CHANNEL_INFO& tInfo)
{
    /* CChanel 정보 채우기 */
    m_iConnectedBoneIndex = tInfo.iConnectedBoneIndex;

    if (FAILED(Ready_KeyFrame(tInfo.vecKeyFrame)))
        return E_FAIL;

    return S_OK;
}

HRESULT CChannel::Ready_KeyFrame(const vector<Key_Frame>& vecKeyFrame)
{
    m_vecKeyFrame.resize(vecKeyFrame.size());

    memcpy(m_vecKeyFrame.data(), vecKeyFrame.data(), sizeof(Key_Frame) * vecKeyFrame.size());

    /* 복사 이거될려나? 해보지뭐 안되면 돌려 */

    return S_OK;
}

Key_Frame CChannel::Make_CurTrackPositionFrame(float fCurrentTrackPosition, uint32& iCurKeyFrameIndex)
{
    KEY_FRAME NewKeyFrame{};

    /* 이경우 내 마지막트랙포지션을 지나간경우 SRT가 그대로 이어져야한다.. */
    if (fCurrentTrackPosition >= m_vecKeyFrame.back().fTrackPosition)
        NewKeyFrame = m_vecKeyFrame.back();
    else /* 아닌경우 선형보관을해야한다. */
    {   /* 가장 최근에 지나친 keyFrame의 인덱스 번호 = 왼쪽, +1 = 오른쪽 */
        /* 내 재생 프레임에 맞는 인덱스 번호를찾아야한다 프레임이 밀렸을떄 DT값에 따라서 내 왼쪽의 인덱스값이 인덱스값이 아니게 되는현상을 방지..*/
        while (fCurrentTrackPosition > m_vecKeyFrame[iCurKeyFrameIndex + 1].fTrackPosition)
            iCurKeyFrameIndex++;


        Key_Frame LK{ m_vecKeyFrame[iCurKeyFrameIndex] }, RK{ m_vecKeyFrame[iCurKeyFrameIndex + 1] };
        /* fTrackPosition=> 음...KeyFrame은 애니매이션의 Frame관련이고  TrackPostion 그 키 프레임 안에 */
        float Ratio = (fCurrentTrackPosition - LK.fTrackPosition) / (RK.fTrackPosition - LK.fTrackPosition);

        XMStoreFloat3(&NewKeyFrame.S, XMVectorLerp(XMLoadFloat3(&LK.S), XMLoadFloat3(&RK.S), Ratio));
        XMStoreFloat4(&NewKeyFrame.R, XMQuaternionSlerp(XMLoadFloat4(&LK.R), XMLoadFloat4(&RK.R), Ratio));
        XMStoreFloat3(&NewKeyFrame.T, XMVectorLerp(XMVectorSetW(XMLoadFloat3(&LK.T), 1.f), XMVectorSetW(XMLoadFloat3(&RK.T), 1.f), Ratio));
    }

    return NewKeyFrame;
}

void CChannel::Update_ConnectedBoneTransformMatrix(float fCurrentTrackPosition, const vector<CBone*>& vecBone , uint32& iCurKeyFrameIndex)
{
    if (vecBone.empty()) return;

    /* 애니매이션이 끝난경우 CurTrackPosition이 0 으로 바뀐다.*/
    if (fCurrentTrackPosition == 0.f)  iCurKeyFrameIndex = 0;

    /* 이렇게 도출된 SRT 로 행렬을 만들어준다. */
    vecBone[m_iConnectedBoneIndex]->Set_TransformMatrix(Make_CurTrackPositionFrame(fCurrentTrackPosition,iCurKeyFrameIndex).GetMatrix());

}

void CChannel::Update_BlendAnimation(uint32 iChangeAnimFrameIndex, float fChangeAnimTrackPosition,const vector<CBone*>& vecBone, const vector<Float4x4>& vecBlendStartBoneMatrix ,float DTLearp)
{
    /* pPreAnimChannel이 가지고 있던 PreAnimIndex를 가지고 계산해준다  */

    KEY_FRAME BlendKeyFrame{};
    Matrix PreBoneMatrix = XMLoadFloat4x4(&vecBlendStartBoneMatrix[m_iConnectedBoneIndex]);

    Vector4 PreS{}, PreR{}, PreT{};
    XMMatrixDecompose(&PreS,&PreR,&PreT,PreBoneMatrix);


    KEY_FRAME ChangeAnimKeyFrame = this->Make_CurTrackPositionFrame(fChangeAnimTrackPosition,iChangeAnimFrameIndex);

    /* SRT 보간 */
    XMStoreFloat3(&BlendKeyFrame.S, XMVectorLerp(PreS,XMLoadFloat3(&ChangeAnimKeyFrame.S), DTLearp));
    XMStoreFloat4(&BlendKeyFrame.R, XMQuaternionSlerp(PreR,XMLoadFloat4(&ChangeAnimKeyFrame.R), DTLearp));
    XMStoreFloat3(&BlendKeyFrame.T, XMVectorLerp(PreT,XMLoadFloat3(&ChangeAnimKeyFrame.T), DTLearp));

    vecBone[m_iConnectedBoneIndex]->Set_TransformMatrix(BlendKeyFrame.GetMatrix());
}

CChannel* CChannel::Create(const CHANNEL_INFO& tInfo)
{
	CChannel* pChannel = new CChannel;

    if (FAILED(pChannel->Initialize(tInfo)))
    {
        Safe_Release(pChannel);
        MSG_BOX("Channel is failed To Create");
        return nullptr;
    }

    return pChannel;
}

void CChannel::Free()
{
	__super::Free();

	/* CChannel::Free() */
    m_vecKeyFrame.clear();
}
