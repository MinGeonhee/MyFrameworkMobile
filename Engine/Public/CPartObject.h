#pragma once
#include "CGameObject.h"

namespace Engine
{
	class CTransform;
	class CContainerObject;
}

NS_BEGIN(Engine)

class ENGINE_DLL CPartObject : public CGameObject
{
public:
	typedef struct PartObject_Desc : public GAMEOBJECT_DESC
	{

		/* 부모의 트랜스폼을 받아오자. */
		const CTransform* PartObjectDesc_pParentTransform{nullptr};
		/* 이 PartObject들을 가지고 있는 Master(주인)을 포인터 형식으로 들고있어주면 좋을거같은데? */
		CContainerObject* PartObjectDesc_pMasterContainerObject{ nullptr };

	}PARTOBJECT_DESC;
protected:
	explicit CPartObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CPartObject(const CPartObject& Proto);
	virtual ~CPartObject() {}
protected:
	HRESULT				Proto_Initialize();
	HRESULT				Clone_Initialize(void* pArg)override;
	virtual HRESULT		Ready_Component(void* pArg);	
public:
	virtual void		Pre_Update(float DT)						override;
	virtual void		Update(float DT)							override;
	virtual void		Late_Update(float DT)						override;
	virtual HRESULT		Add_RenderGroup()							override;
	virtual HRESULT		Render(FD CCameraObjectBase* pRenderCam)	override;

public:
	CContainerObject*	Get_MasterContainerObject() const { return m_pMasterContainerObject; }
	virtual bool		Get_IsInCamArea(const Float4& vRenderCamAreaXZ)override;
protected:
	/* 부모행렬을 가지고 있을 변수 */
	const Float4x4*	  m_pMasterWorldMatrix;
	Float4x4		  m_matCombinedWorld;
	CContainerObject* m_pMasterContainerObject;

public:
	void		      SetUp_CombinedWorldMatrix(const XMatrix_1& ParentWorldMatrix);

public:
	virtual void Free();
};

NS_END

