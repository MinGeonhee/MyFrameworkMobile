#include "CAnimation.h"
#include "CChannel.h"
#include "CBone.h"

CAnimation::CAnimation()
    : m_fTickPerSecond{}, m_fDuration{}, m_fCurTrackPosition{}, m_vecChannel{}
    , m_szName{}, m_ePlayType{ ANIMATION_PLAYTYPE::ONCE }, m_vecCurKeyFrameIndex{}, m_isAnimationEnd{false}
{
}

/* Animatino 깊은복사 Resource에서 가져올때 원본이아닌, 복사본으로 들고 오도록하자.? */
CAnimation::CAnimation(const CAnimation& Proto)
    : m_fCurTrackPosition(Proto.m_fCurTrackPosition) , m_fDuration(Proto.m_fDuration), m_fTickPerSecond(Proto.m_fTickPerSecond)
    , m_vecChannel(Proto.m_vecChannel), m_ePlayType(Proto.m_ePlayType), m_vecCurKeyFrameIndex{ Proto.m_vecCurKeyFrameIndex }
    , m_isAnimationEnd{ Proto.m_isAnimationEnd }
{
    strcpy_s(this->m_szName, MAX_PATH ,Proto.m_szName);

    for (auto& Channel : m_vecChannel)
        Safe_AddRef(Channel);
}

CAnimation::~CAnimation()
{
}

HRESULT CAnimation::Initialize(const ANIM_INFO& tInfo)
{
    /* 기본 Animation 정보  */
    strcpy_s(m_szName,MAX_PATH,tInfo.szName);
    m_fTickPerSecond =  tInfo.fTickPerSecond;
    m_fDuration = tInfo.fDuration;
    m_vecCurKeyFrameIndex.resize(tInfo.iCountChannel);


    if (FAILED(Ready_Channel(tInfo.vecChannelInfo)))
        return E_FAIL;


    return S_OK;
}

HRESULT CAnimation::Ready_Channel(const vector<CHANNEL_INFO>& vecChannelInfo)
{
    for (const auto& ChannelInfo : vecChannelInfo)
    {
        /* Channel 을 생성 */
        CChannel* pChannel = CChannel::Create(ChannelInfo);
        NULLPTR_RETURN_EFAIL(pChannel);
        m_vecChannel.push_back(pChannel);
    }

    return S_OK;
}

void CAnimation::ResetAnimation()
{
    m_fCurTrackPosition = 0.f;
    memset(m_vecCurKeyFrameIndex.data(), 0, sizeof(uint32) * m_vecCurKeyFrameIndex.size());
}

float CAnimation::Get_AnimProgress()
{
    return m_fCurTrackPosition / m_fDuration;
}

uint32 CAnimation::Get_CurAnimKeyFrameIndex()
{
    return m_vecCurKeyFrameIndex[0];
}

void CAnimation::Update_TransformMatrix( float fTickPerSecond ,float DT,const vector<CBone*>& vecBone , bool& isAnimEnd)
{
    /* 현재 내 트랙(재생되는 애니매이션의 위치가 어디인지) = 현재 시간에서 * 나의 애니매이션 속도...  */
    m_fCurTrackPosition += DT * fTickPerSecond * 2.f;

    /* 현재 내 트랙포지션이 Druation(전체길이) 보다 많을경우 애니매이션 재생이 끝난상태이다. */
    /* 애니매이션이 끝났다는 판정은 여기서해주고있는 입장이다보니까... 이게 좀 애매해지네...음
        어케짜줘야 이쁘게될까....일단  */
    if (m_fCurTrackPosition >= m_fDuration)
    {
       
        switch (m_ePlayType)
        {
        case Engine::ANIMATION_PLAYTYPE::ONCE:
            m_isAnimationEnd = true;
            isAnimEnd = true;
            ResetAnimation();
            return; // Once 애니매이션을 끝으로 다른애니매이션이 재생될예정이기 때문에 아래 Update문을 들어가지않고 return 해준다.
        case Engine::ANIMATION_PLAYTYPE::LASTSTOP:
            m_fCurTrackPosition = m_fDuration;
            m_isAnimationEnd = true;
            isAnimEnd = true;
            //ResetAnimation();
            break;
        case Engine::ANIMATION_PLAYTYPE::LOOP: 
            m_isAnimationEnd = true;
            isAnimEnd = true;
            ResetAnimation();
            break;

        default:
            break;
        }
    }
    else
    {
        m_isAnimationEnd = false;
        isAnimEnd = false;
    }

    uint32 i = 0;
    for (const auto& Channel : m_vecChannel)
        Channel->Update_ConnectedBoneTransformMatrix(m_fCurTrackPosition,vecBone,m_vecCurKeyFrameIndex[i++]);
}

void CAnimation::Update_BlendAnimation(const vector<CBone*>& vecOriginBone, const vector<Float4x4>& vecBlendStartBoneMatrix, float fLerpTime)
{
    /* 겹치는 Index 를 뽑아내야한다 아니면....pPreAnimation을가져와서 PreAnimation->Channe에 접근.. 다음 바껴야할 모든 본에 접근을해서 SRT를 구한다...*/
    /* 어쩔 수 없이 SRT를 가져와야하긴해...  */

    /* Channel에 건내주고 Index가 같으면 Index를 어케해줘야할거같ㅇ느데 */
    uint32 CurChIndex{}, PreChIndex{};

    /* 애니매이션이 끝나고나서 진행되는 선형보간 애니매이션의 경우에는 트랙포지션 마지막 KeyFrame을 기준으로 새로운 애니매이선의 Index번호와 조합해서 진행이들어가는게맞다 이렇게 할 게 */
    /* 현재 마지막 Bone Pose Matrix를 vector<Bone>에 전부 저장중, 변경 후 애니매이션에서 사용할 Bone정보와 Connected된 Bone의 */
    for (auto& Channel : m_vecChannel)
    {
        Channel->Update_BlendAnimation(m_vecCurKeyFrameIndex[CurChIndex],m_fCurTrackPosition,vecOriginBone,vecBlendStartBoneMatrix,fLerpTime);
        CurChIndex++;
    }

    /* 이렇게 되면 겹치는 Bone들에 대해서는 업데이트가 돌아갈 에정 겹치지 않는 본들은 업데이트가 되지 않은 채로 남아있을듯 */
}   

CAnimation* CAnimation::Create(const ANIM_INFO& tInfo)
{
    CAnimation* pAnim = new CAnimation;

    if (FAILED(pAnim->Initialize(tInfo)))
    {
        Safe_Release(pAnim);
        MSG_BOX("Animation is failed To Create");
        return nullptr;
    }


    return pAnim;
}



CAnimation* CAnimation::Clone()
{
    return new CAnimation(*this);
}


void CAnimation::Free()
{
    __super::Free();

    /* CAnimation::Free() */
    for (auto& Channel : m_vecChannel)
        Safe_Release(Channel);
    m_vecChannel.clear();
}




#ifdef _DEBUG
void CAnimation::ImGui_Update()
{

}
#endif // _DEBUG
