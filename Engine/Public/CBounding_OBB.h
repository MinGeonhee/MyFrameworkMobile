#pragma once
#include "CBounding.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBounding_OBB final : public CBounding
{
public:
	typedef struct Bounding_OBB_Desc : public CBounding::BOUDING_DESC
	{
		Float3 vLocalExtents  { 1.f,1.f,1.f };
		Float3 vLocalRotation { 0.f,0.f,0.f };
	}BOUNDING_OBB_DESC;
private:
	explicit CBounding_OBB(ComPtr<Device>pDevice, ComPtr<DeviceContext>pContext);
	virtual ~CBounding_OBB() {}
private:
	HRESULT					Initialize(void* pArg);
public:
	virtual bool			Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)		override;
	virtual bool			Intersects(CBounding* pOtherBounding, COLLISION_INFO* pOutInfo, COLLIDER_TYPE eType)override;
	virtual void			Update_World(const Float4x4* pWorldMatrix)						override;
	virtual void			Update_World(const XMatrix_1& WorldMatrix)						override;
	virtual	Vector4			Get_WorldPosition()												override;
#ifdef _DEBUG
	virtual bool			ImGuiUpdate() override;
	virtual void			Save_Bounding(nlohmann::ordered_json& json) override;
	virtual HRESULT			Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)override;
	virtual HRESULT			Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor) override;

#endif
public:
	BoundingOrientedBox*	Get_LocalBoundingOrientedBox() const { return m_pLocalBoundingOrientedBox; }
	BoundingOrientedBox*	Get_WorldBoundingOrientedBox() const { return m_pWorldBoundingOrientedBox; }
private:
	BoundingOrientedBox*	m_pLocalBoundingOrientedBox;
	BoundingOrientedBox*	m_pWorldBoundingOrientedBox;

#ifdef _DEBUG
	Float3					m_vRotation{};
#endif

public:
	static  CBounding_OBB*		Create(void* pArg,ComPtr<Device>pDevice, ComPtr<DeviceContext>pContext);
	virtual void				Free() override;
};

NS_END
