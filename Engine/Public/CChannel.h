#pragma once
#include "CBase.h"


namespace Engine
{
	class CBone;
}


NS_BEGIN(Engine)

class CChannel : public CBase
{
public:
	explicit CChannel();
	explicit CChannel(const CChannel& Proto);
	virtual ~CChannel();
private:
	HRESULT		Initialize(const CHANNEL_INFO& tInfo);
	HRESULT		Ready_KeyFrame(const vector<Key_Frame>& vecKeyFrame);
public:
	uint32		Get_FrameCount()			const { return Cast_Uint(m_vecKeyFrame.size()); }
	uint32		Get_ConnectedBoneIndex()	const {return m_iConnectedBoneIndex;}

	Key_Frame   Make_CurTrackPositionFrame(float fCurrentTrackPosition , uint32& iCurKeyFrameIndex);

	void		Update_ConnectedBoneTransformMatrix(float fCurrentTrackPosition, const vector<CBone*>& vecBone, uint32& iCurKeyFrameIndex);
	void		Update_BlendAnimation(uint32 iChangeAnimFrameIndex, float fChangeAnimTrackPosition,const vector<CBone*>& vecOriginBone,const vector<Float4x4>& vecBlendStartBoneMatrix, float DTLearp);

	/* Get함수가 2개 내 처음 Index의 KeyFrame을 가져가야함 */


	const		Key_Frame& Get_KeyFrame(uint32 iKeyFrameIndex)  const { return m_vecKeyFrame[iKeyFrameIndex];}


private:
	/* 채널은 Anmiaition과 연관된 뼈들의 Index번호와 그에맞는 행렬 값을 저장시켜놓는다.*/
	/* 즉 채널은 뼈한개를 의미하면서 그뼈에 어떠한 애니매이션에 맞는 정보를 줄건지에대한 정보와함꼐 저장된다 */
	/* 연관된 Bone의 Model 에서 들고있는 Index번호 */
	int32					m_iConnectedBoneIndex;
	
	/* 그래서 이뼈가 프레임별로 어떤식으로 변할지에대한 SRT 값을 프레임 개수대로 모아둠 */
	vector<Key_Frame>		m_vecKeyFrame;

public:
	static CChannel* Create(const CHANNEL_INFO& tInfo);
	virtual void Free()override;
};
NS_END

