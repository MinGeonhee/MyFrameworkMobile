#pragma once
#include "CBounding.h"


NS_BEGIN(Engine)

class CBounding_Sphere final: public CBounding
{
public:
	typedef struct Bounding_Sphere_Desc : public CBounding::BOUDING_DESC
	{
		float fRadius{0.5f};
	}BOUNDING_SPHERE_DESC;
private:
	explicit CBounding_Sphere(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CBounding_Sphere() {}
protected:
	HRESULT	Initialize(void* pArg);
public:
	virtual bool		  Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)	 override;
	virtual bool		  Intersects(CBounding* pOtherBounding, COLLISION_INFO* pOutInfo ,COLLIDER_TYPE eType)	 override;
	virtual void		  Update_World(const Float4x4* pWorldMatrix)					 override;
	virtual void		  Update_World(const XMatrix_1& WorldMatrix)					 override;
	virtual	Vector4		  Get_WorldPosition()												override;

private:
	bool				  MyIntersect_Sphere(const BoundingSphere* pOtherBounding,COLLISION_INFO* pOutInfo);
	bool				  MyIntersect_AABB(const BoundingBox* pOtherBounding,COLLISION_INFO* pOutInfo);
#ifdef _DEBUG
	virtual bool		  ImGuiUpdate() override;
	void				  Save_Bounding(nlohmann::ordered_json& json) override;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)override;
	virtual HRESULT		  Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor)override;

#endif

public:
	BoundingSphere* Get_LocalBoundingSphere() const { return m_pLocalBoundingSphere; }
	BoundingSphere* Get_WorldBoundingSphere() const { return m_pWorldBoundingSphere; }
private:
	BoundingSphere* m_pLocalBoundingSphere;
	BoundingSphere* m_pWorldBoundingSphere;
public:
	static CBounding_Sphere* Create(void* pArg, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void			Free()override;
};

NS_END

