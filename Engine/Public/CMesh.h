#pragma once
#include "CBase.h"

namespace Engine
{
	class CTexture;
	class CShader;
	class CVIBuffer;
	class CVIBuffer_Mesh;
	class CMaterial;
	class CBone;
}

NS_BEGIN(Engine)

class ENGINE_DLL CMesh :public CBase
{
private:
	explicit CMesh(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CMesh(const CMesh& Proto);
	virtual ~CMesh() {}
private:

	HRESULT		 Initialilze_Anim(const string& strMeshName, const MeshConnectedBoneInfo& tMeshConnectedBoneInfo,const wstring& strMaterialName,const MTL_PATH& strMtlPath, const VI_INFO& tVIInfo);
	HRESULT		 Initialilze_NonAnim(const string& strMeshName,const wstring& strMaterialName,const MTL_PATH& strMtlPath, const VI_INFO& tVIInfo, const XMatrix_1& matTransform);
	HRESULT		 Ready_Buffer(MODEL_TYPE _eModelType, VI_INFO tVIInfo, const XMatrix_1& matTransform = XMMatrixIdentity());
	HRESULT		 Ready_Material(const wstring& _strMaterialName,const MTL_PATH& _wstrMtlPath);
public:
	HRESULT		 Set_GPU_Material(CShader* pShader);
	HRESULT		 Set_GPU_ConnectedBoneMatrix(CShader* pShader,const string& strName, const vector<CBone*>& vecBone);
	HRESULT		 Bind_Resource();
	HRESULT		 Draw(class CShader* pShader, const string& strPathKey , uint32 iIndex = 0);
public:
	HRESULT		 Set_Material_Index(MATERIAL_TYPE _eType, uint32 _iIndex);
public:
	bool		 Get_IsActive() CONST { return m_isActive; }
	string		 Get_Name()	   CONST { return m_strName; }
	wstring		 Get_MaterialName() CONST;
	CVIBuffer*   Get_Buffer()		CONST { return m_pBuffer; }
	CGameObject* Get_Owner()		CONST { return m_pOwner; }
	CMaterial*	 Get_Material()		CONST { return m_pMaterial; }
public:
	void		Set_Owner(class CGameObject* pOwner);
	void		Set_Active(bool isActive) { m_isActive = isActive; }
	void		Set_Name(const string& strName) { m_strName = strName; }

private:

	CGameObject*			m_pOwner;
	bool					m_isActive;
	string					m_strName;


	/* 이 매쉬가 영향을 받는 Bone의 개수 */
	uint32					m_iCountConnectedBone;
	/* 이 매쉬가 영향을 받는 Bone의 인덱스 , 이건 부모의 vector<Bone*>안에 들어 있는 Bone의 인덱스 번호이다.*/
	vector<uint32>			m_vecConnectedBoneIndex;
	/* 특졍 뼈를 정점에게 적용할떄 추가 보정을 거쳐서 정점에게 상태를 전달할 수 있도록 한다. */
	vector<Float4x4>		m_vecConnectedBoneOffsetMatrix;
	/*  이 매쉬가 영향을 받는 Bone의 인덱스로부터 Bone의 매트릭스를 받아서 GPU에게 던져준다.*/
	Float4x4				m_BoneMatrices[g_iMaxNumBones] = {};



private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;

	CMaterial*				m_pMaterial;

	CVIBuffer*				m_pBuffer;
public:
	static  CMesh*  Create_Anim(const string& strMeshName, const MeshConnectedBoneInfo& tMeshConnectedBoneInfo ,const wstring& wstrMaterialName, const MTL_PATH& strMtlPath, VI_INFO tVIInfo , ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	static	CMesh*	Create_NonAnim(const string& strMeshName ,  const wstring& wstrMaterialName ,const MTL_PATH& strMtlPath, VI_INFO tVIInfo , const XMatrix_1& matTransform,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void	Free();

public:
	CMesh*			Clone();
};


NS_END;

