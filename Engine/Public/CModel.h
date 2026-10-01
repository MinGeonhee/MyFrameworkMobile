#pragma once
#include "CResource.h"

namespace Engine
{
	class CVIBuffer_Mesh;
	class CMesh;
	class CBone;
	class CShader;
	class CAnimation;
}


/* 모델을 상속으로 NoneAnim or Anim 으로 나눌지 그냥 타입별 진행해서 할지... */
/* 모델을 상속으로내려서...  */


NS_BEGIN(Engine)


class ENGINE_DLL CModel final : public CResource
{
protected: 
	explicit CModel(ComPtr<Device>  pDevice, ComPtr<DeviceContext> pContext);
	explicit CModel(const CModel& Proto);
	virtual ~CModel() {}
protected:
	virtual HRESULT		Initailize(const wstring& _wstrModelInfoPath, const XMatrix_1& PreTransformMatrix);
	virtual HRESULT		Copy_Initialize(void* pArg);


private:
	/* Json으로 정보를 받아오자 */
	HRESULT					Ready_AnimModel(const XMatrix_1& PreTransformMatrix,const nlohmann::ordered_json& ModelJson);
	HRESULT					Ready_NonAnimModel(const XMatrix_1& PreTransformMatrix,const nlohmann::ordered_json& ModelJson);

	HRESULT					Ready_NonAnimMesh(const XMatrix_1& PreTransformMatrix,uint32 iCountMesh,CONST nlohmann::ordered_json& Meshjson);
	HRESULT					Ready_AnimMesh(uint32 iCountMesh, CONST nlohmann::ordered_json& Meshjson);
private:
	HRESULT					Ready_Animation(uint32 iCountAnimation , const string& strAnimPath);

protected:
	CMesh*					Find_Mesh(const string& _strName);
	HRESULT					Ready_Bone(uint32 iCountBone, const wstring& _wstrBonePath, const XMatrix_1& PreTransformMatrix);
 
public:
	HRESULT					Set_GPU_Material(CShader* pShader, int32 iMeshIndex);
	HRESULT					Set_GPU_ConnectBoneMatrix(CShader* pShader, const string& strName , int32 iMeshIndex);
public:
	const Float4x4*			Get_BoneMatrix(const string& strName) const;
	const Float4x4*			Get_CombinedBoneMatrix(const string& strName) const;
	int32					Get_BoneIndex(const string& strName)  const;
public:
	HRESULT					Set_Material_Index( CONST string& _strMeshName,  MATERIAL_TYPE _eType, uint32 _iIndex);
	wstring					Get_Material_Name(CONST string& _strMeshName) const;
public:
	void					Reset_Animation(uint32 iAnimIndex);
	void					Set_IsAnimationEnd(uint32 iAnimIndex, bool isEnd);
	void					Set_ModelName(const wstring& _wstrName) { m_wstrModelName = _wstrName; }
	void					Set_MeshActive( const string& _strMeshName , bool isActive);
	void					Set_MeshActive( uint32 iMeshIndex , bool isActive);
	void					Set_IdleAnimationIndex(uint32 iIdleIndex) { m_iIdleIndex = iIdleIndex; }
	void					Set_Owner(FD CGameObject*pOwner);
	void					Set_IdleAnimIndex(uint32 iIndexIdle) { m_iIdleIndex = iIndexIdle; }
public:
	CGameObject*			Get_Owner() const { return m_pOwner; }
	bool					Get_MeshActive( const  string& _strMeshName);
	wstring					Get_ModelName() const { return m_wstrModelName; }
	uint32					Get_MeshCount() const { return Cast_Uint(m_umapMesh.size());}
	const vector<CMesh*>&	Get_Meshs() const { return m_vecMesh; }
	CMesh*					Get_Mesh(const string& strMeshName);
	uint32					Get_AnimationCount() const { return Cast_Uint(m_vecAnimation.size()); }
	bool					Get_IsAnimationEnd(uint32 iAnimIndex) const;
	float					Get_AnimProgress();
	uint32					Get_AnimCurFrameIndex();
public:
#ifdef _DEBUG
	vector<string>			Get_MeshNames();
	virtual void			ImGui_Update() override;
#endif // _DEBUG

public:
	bool					Play_Animation(float fTickPerSecond,float DT);
	void					Change_Animation(uint32 iSetAnimIndex, ANIMATION_PLAYTYPE ePlayType);
	void					Update_BlendAnimation(float DT);
	void					Apply_BoneCombineTrasnform(const string& strStartBoneName, const string& strEndBoneName,const XMatrix_1& CombineMatrix);

	HRESULT					Draw(CShader* _pShader, const string& strName ,uint32 iMeshIndex);
protected:
	CGameObject*							m_pOwner;

protected:
	wstring									m_wstrModelName;
	/* Mesh 키값으로 찾기용 */
	unordered_map< string , CMesh*>			m_umapMesh;

	/* PreTransformMatrix */
	Float4x4								m_matPreTransform;

	/* 순회용 */
	vector<CMesh*>							m_vecMesh;

	/* Model Type */
	MODEL_TYPE								m_eModelType;

	/* Bone */
	vector<CBone*>							m_vecBone;


	/* Animatino 관련 */
	uint32									m_iIdleIndex;
	uint32									m_iPreAnimIndex;
	bool									m_isAnimEnd;
	bool									m_isBlendAnimEnd;
	uint32									m_iCurrentAnimIndex;
	vector<CAnimation*>						m_vecAnimation;

	/*  */
	vector<Float4x4>						m_vecBlendStartBoneMatrices;
	float									m_fLerpAccTime;
	float									m_fLerpTime;


#ifdef _DEBUG

	uint32									m_iSelectAnim;

#endif // _DEBUG


public:
	static  CModel*		Create(const wstring& _wstrModelInfoPath, const XMatrix_1& PreTransformMatrix ,ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual CResource*	Copy_Resource(void* pArg);
	virtual void Free() override;
};

NS_END