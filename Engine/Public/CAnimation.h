#pragma once
#include "CBase.h"

namespace Engine
{
	class Model;
	class CBone;
	class CChannel;
}

NS_BEGIN(Engine)

class CAnimation : public CBase
{
public:
	explicit CAnimation();
	explicit CAnimation(const CAnimation& Proto);
	virtual ~CAnimation();
private:
	HRESULT		Initialize(const ANIM_INFO& tInfo);

	HRESULT		Ready_Channel(const vector<CHANNEL_INFO>& vecChannelInfo);
public:

	void				Set_IsAnimationEnd(bool isEnd) { m_isAnimationEnd = isEnd; }
	void				SetPlayType(ANIMATION_PLAYTYPE ePlayType) { m_ePlayType = ePlayType; }
	void				ResetAnimation();

public:
	float				Get_AnimProgress();
	uint32				Get_CurAnimKeyFrameIndex();
	bool				Get_IsAnimationEnd() const { return m_isAnimationEnd; }
	uint32				Get_ChannelCount() const { return static_cast<uint32>(m_vecChannel.size());}
	
	string				Get_AnimationName() const { return string(m_szName); }
	
	void				Update_TransformMatrix(float fTickPerSecond,float DT, const vector<CBone*>& vecBone, bool& isEnd);

	void				Update_BlendAnimation(const vector<CBone*>& vecOriginBone,const vector<Float4x4>& vecBlendStartBoneMatrix,float fLerpTime);


#ifdef _DEBUG
public:

	void				ImGui_Update();

#endif // _DEBUG



private:
	char				m_szName[MAX_PATH];
	
	vector<CChannel*>	m_vecChannel;

	vector<uint32>		m_vecCurKeyFrameIndex;

	ANIMATION_PLAYTYPE	m_ePlayType;
	
	/* 이 애니매이션이 바껴야할떄 선형보관 값을 어케할것인가...  */
	float				m_fTickPerSecond;    // 애니매이션 재생 속도
	float				m_fDuration;		 // 요개이제 애니매이션의 전체 재생 길이 
	float				m_fCurTrackPosition; // 현재 이 애니매이션의 재생되고있는 Track 포지션 깊은복사를해서 속도가 빨라지는 현상을 막자.
	
	bool				m_isAnimationEnd;

public:
	static CAnimation* Create(const ANIM_INFO& tInfo);
	CAnimation*	 	   Clone();
	virtual void Free()override;
};

NS_END