#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CBounding abstract : public CBase
{
public:
	typedef struct Bounding_Desc : public COLLIDERCOM_DESC
	{
		Float3		vLocalCenter{0.f,0.f,0.f};
	}BOUDING_DESC;
protected:
	explicit CBounding(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual ~CBounding();
protected:
	HRESULT	Initialize();
public:
	/* 이 Intersect_Ray Type */
	virtual bool		  Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)		PURE;
	virtual bool		  Intersects(CBounding* pOtherBounding, COLLISION_INFO* pOutInfo, COLLIDER_TYPE eType)		PURE;
	virtual void		  Update_World(const Float4x4* pWorldMatrix)						PURE;
	virtual void		  Update_World(const XMatrix_1& WorldMatrix)						PURE;
	virtual	Vector4		  Get_WorldPosition()												PURE;
#ifdef _DEBUG
	virtual bool		  ImGuiUpdate() PURE;
	virtual void		  Save_Bounding(nlohmann::ordered_json& json) PURE;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch , COLLIDER_STATE eColliderState)PURE;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch , const Float4 vColor)PURE;
#endif 

protected:
	ComPtr<Device>		  m_pDevice;
	ComPtr<DeviceContext> m_pDeviceContext;
public:
	void Free() override;
};

NS_END
