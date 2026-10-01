#pragma once
#include "CBase.h"

NS_BEGIN(Engine)



class ENGINE_DLL CBone final: public CBase
{
private:
	explicit CBone();
	explicit CBone(const CBone& Proto);
	virtual ~CBone();
private:
	HRESULT	 Initialize(const BONE_INFO& tBoneInfo);
public:
	const	BONE_INFO& Get_BoneInfo() CONST { return m_tInfo;}
	void	Update_CombinedTransformationMatrix(const vector<CBone*>& vecBone, const XMatrix_1& PreTransformMatrix);


	const	Float4x4* Get_BoneMatrixPtr() const { return &m_tInfo.matTransform; }
	const   Float4x4* Get_CombinedTransformationMatrixPtr() const { return &m_tInfo.matCombinedTransformation; }
	const	Float4x4&	Get_CombinedTransformationMatrix_Store() const { return m_tInfo.matCombinedTransformation;}
	Matrix	Get_CombinedTransformationMatrix_Cal() const { return XMLoadFloat4x4(&m_tInfo.matCombinedTransformation);}
	void	Set_TransformMatrix(const XMatrix_1& Matrix) { XMStoreFloat4x4(&m_tInfo.matTransform, Matrix); }
	void	Set_TransformMatrix(const Float4x4& Matrix) { m_tInfo.matTransform = Matrix; }

	void	Apply_BoneCombineTrasnform(const XMatrix_1& CombineMatrix);

private:

	BONE_INFO		m_tInfo;

public:
	static  CBone* Create(const BONE_INFO& tBoneInfo);
	CBone*		   Clone();
	virtual void Free() override;
};
NS_END
