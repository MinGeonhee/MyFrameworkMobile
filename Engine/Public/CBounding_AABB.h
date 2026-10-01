#pragma once
#include "CBounding.h"

NS_BEGIN(Engine)

class CBounding_AABB final: public CBounding
{
public:
	typedef struct Bounding_AABB_Desc : public CBounding::BOUDING_DESC
	{
		Float3 vLocalExtents{ 1.f,1.f,1.f };
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CBounding_AABB() {}
private:
	HRESULT	Initialize(void* pArg);
public:
	virtual bool		  Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)	override;
	virtual bool		  Intersects(CBounding* pOtherBounding, COLLISION_INFO* pOutInfo, COLLIDER_TYPE eType)	override;
	virtual void		  Update_World(const Float4x4* pWorldMatrix)					override;
	virtual void		  Update_World(const XMatrix_1& WorldMatrix)					override;
	virtual	Vector4		  Get_WorldPosition()												override;


#ifdef _DEBUG
	virtual bool		  ImGuiUpdate()															override;
	virtual void		  Save_Bounding(nlohmann::ordered_json& json)							override;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)override;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor)override;

#endif
private:
	bool				  MyIntersect_Sphere(const BoundingSphere* pOtherBounding, COLLISION_INFO* pOutInfo);
	bool				  MyIntersect_AABB(const BoundingBox* pOtherBounding, COLLISION_INFO* pOutInfo);
public:
	BoundingBox*		Get_LocalBoundingBox() const { return m_pLocalBoundingBox; }
	BoundingBox* Get_WorldBoundingBox() const { return m_pWorldBoundingBox; }
private:
	BoundingBox* m_pLocalBoundingBox;
	BoundingBox* m_pWorldBoundingBox;
public:
	static CBounding_AABB*  Create(void* pArg,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void			Free()override;
};

NS_END