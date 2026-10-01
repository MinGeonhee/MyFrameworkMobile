#include "CBounding_Sphere.h"
#include "CBounding_AABB.h"
#include "CBounding_OBB.h"

#include "DebugDraw.h"

CBounding_Sphere::CBounding_Sphere(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CBounding(pDevice,pContext),m_pLocalBoundingSphere(nullptr) , m_pWorldBoundingSphere(nullptr)
{
}


HRESULT	CBounding_Sphere::Initialize(void* pArg)
{
	CBounding_Sphere::BOUNDING_SPHERE_DESC* pDesc = static_cast<CBounding_Sphere::BOUNDING_SPHERE_DESC*>(pArg);


	m_pLocalBoundingSphere = new BoundingSphere(pDesc->vLocalCenter,pDesc->fRadius);
	m_pWorldBoundingSphere = new BoundingSphere(*m_pLocalBoundingSphere);



	return S_OK;
}


void CBounding_Sphere::Update_World(const Float4x4* pWorldMatrix)
{
	m_pLocalBoundingSphere->Transform(*m_pWorldBoundingSphere, XMLoadFloat4x4(pWorldMatrix));
}

void CBounding_Sphere::Update_World(const XMatrix_1& WorldMatrix)
{
	m_pLocalBoundingSphere->Transform(*m_pWorldBoundingSphere,WorldMatrix);
}

Vector4 CBounding_Sphere::Get_WorldPosition()
{
	return  XMVectorSetW(XMLoadFloat3(&m_pWorldBoundingSphere->Center), 1.f);
}

bool	CBounding_Sphere::Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)
{
	return m_pWorldBoundingSphere->Intersects(XMLoadFloat3(&tRayInfo.vRayOrigin), XMLoadFloat3(&tRayInfo.vRayDir),pRayOut->fDistance);
}
bool	CBounding_Sphere::Intersects(CBounding* pOtherBounding,COLLISION_INFO* pOutInfo, COLLIDER_TYPE eType)
{
	if (pOtherBounding == nullptr) return false;

	switch (eType)
	{
	case COLLIDER_TYPE::SPHERE: return MyIntersect_Sphere(static_cast<CONST CBounding_Sphere*>(pOtherBounding)->Get_WorldBoundingSphere(),pOutInfo);
		break;
	case COLLIDER_TYPE::AABB:	return MyIntersect_AABB(static_cast<CONST CBounding_AABB*>(pOtherBounding)->Get_WorldBoundingBox(), pOutInfo);
		break;
	case COLLIDER_TYPE::OBB	:	return m_pWorldBoundingSphere->Intersects(*static_cast<CONST CBounding_OBB*>(pOtherBounding)->Get_WorldBoundingOrientedBox());
		break;
	default:
		break;
	}

	return false;
}




bool CBounding_Sphere::MyIntersect_Sphere(const BoundingSphere* pOtherBounding, COLLISION_INFO* pOutInfo)
{
	if(pOtherBounding == nullptr) return false;

	bool isCollision{};


	XMVECTOR vCenterA = XMLoadFloat3(&m_pWorldBoundingSphere->Center);
	XMVECTOR vCenterB = XMLoadFloat3(&pOtherBounding->Center);
	XMVECTOR vDir = XMVectorSubtract(vCenterB, vCenterA);

	float Distance = GetLengthVector(vDir));
	float DistancePlusRaius = this->m_pWorldBoundingSphere->Radius + pOtherBounding->Radius;

	/* 두 반지름의 길이의 합 > 두 중점사이의 거리라면 충돌이 난거다 */
	if (DistancePlusRaius >= Distance)
	{
		/* B-A의 단위벡터를 구해주면된다 */
		XMStoreFloat3(&pOutInfo->vSrcPenetrationDir, -XMVector3Normalize(vDir));
		XMStoreFloat3(&pOutInfo->vDstPenetrationDir, +XMVector3Normalize(vDir));
		/* 깊이는...두 구체 반지름의 합 - 두구체 중심 거리 */
		pOutInfo->fDepth = DistancePlusRaius - Distance;
		return true;
	}
	else
		return false;
}


bool CBounding_Sphere::MyIntersect_AABB(const BoundingBox* pOtherBounding, COLLISION_INFO* pOutInfo)
{
	XMVECTOR SphereCenter = XMLoadFloat3(&this->m_pWorldBoundingSphere->Center);
	XMVECTOR SphereRadius = XMVectorReplicatePtr(&this->m_pWorldBoundingSphere->Radius);

	XMVECTOR BoxCenter = XMLoadFloat3(&pOtherBounding->Center);
	XMVECTOR BoxExtents = XMLoadFloat3(&pOtherBounding->Extents);

	XMVECTOR BoxMin = XMVectorSubtract(BoxCenter, BoxExtents);
	XMVECTOR BoxMax = XMVectorAdd(BoxCenter, BoxExtents);
	// Compute d for each dimension.
	XMVECTOR LessThanMin = XMVectorLess(SphereCenter, BoxMin);
	XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxMax);

	/* 박스위 Min 좌표보다 작거나 같은 */
	XMVECTOR MinDelta = XMVectorSubtract(SphereCenter, BoxMin);
	/* 마찬가지로 Box의 Max좌표 보다 같다 */
	XMVECTOR MaxDelta = XMVectorSubtract(SphereCenter, BoxMax);


	XMVECTOR d = XMVectorZero();
	d = XMVectorSelect(d, MinDelta, LessThanMin);
	d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

	// Use a dot-product to square them and sum them together.
	XMVECTOR d2 = XMVector3Dot(d, d);

	if (XMVector3LessOrEqual(d2, XMVectorMultiply(SphereRadius, SphereRadius)))
	{
		/* 가장가까운점 */
		XMVECTOR ClosestPoint = XMVectorClamp(SphereCenter, BoxMin, BoxMax);

		XMVECTOR vDir = SphereCenter - ClosestPoint;
		float fLength = XMVectorGetX(XMVector3Length(vDir));

		XMVECTOR vNormalDir = XMVector3Normalize(vDir);

		XMStoreFloat3(&pOutInfo->vSrcPenetrationDir, vNormalDir);
		XMStoreFloat3(&pOutInfo->vDstPenetrationDir, -vNormalDir);
		pOutInfo->fDepth = XMVectorGetX(SphereRadius) - fLength;
		return true;
	}
	else
		return false;

	return false;
}





CBounding_Sphere* CBounding_Sphere::Create(void* pArg, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CBounding_Sphere* pBounding = new CBounding_Sphere(pDevice, pContext);
	if(FAILED(pBounding->Initialize(pArg)))
	{
		Safe_Release(pBounding);
		MSG_BOX(" Bounding Sphere is failed to Create ");
		return nullptr;
	}

	return pBounding;
}




void CBounding_Sphere::Free()
{
	/* CBounding::Free() */
	__super::Free();

	Safe_Delete(m_pLocalBoundingSphere);
	Safe_Delete(m_pWorldBoundingSphere);
}







#ifdef _DEBUG
bool CBounding_Sphere::ImGuiUpdate()
{
	bool isAction{ false };

	/* Center */
	ImGui::Text("--- Center --- ");
	if(ImGui::InputFloat("X##1", &m_pLocalBoundingSphere->Center.x, 0.1f))  { isAction = true;}
	if(ImGui::InputFloat("Y##1", &m_pLocalBoundingSphere->Center.y, 0.1f))  { isAction = true;}
	if (ImGui::InputFloat("Z##1", &m_pLocalBoundingSphere->Center.z, 0.1f)) { isAction = true;}

	/* Radius */
	ImGui::Text("--- Radius --- ");
	if (ImGui::InputFloat("Radius##2", &m_pLocalBoundingSphere->Radius, 0.1f)) { isAction=true;}

	return isAction;
}
void CBounding_Sphere::Save_Bounding(nlohmann::ordered_json& json)
{
	json["Center"] = { { "X" , m_pLocalBoundingSphere->Center.x} , {"Y", m_pLocalBoundingSphere->Center.y},{"Z", m_pLocalBoundingSphere->Center.z} };
	json["Radius"] = m_pLocalBoundingSphere->Radius;
}

HRESULT CBounding_Sphere::Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)
{
	switch (eColliderState)
	{
	case Engine::COLLIDER_STATE::ACTIVE:		DX::Draw(pBatch, *m_pWorldBoundingSphere, XMVectorSet(0.f, 0.f, 1.f, 1.f)); break;
	case Engine::COLLIDER_STATE::NON_ACTIVE:	DX::Draw(pBatch, *m_pWorldBoundingSphere, XMVectorSet(0.f, 1.f, 0.f, 1.f));break;
	case Engine::COLLIDER_STATE::COLLISION:		DX::Draw(pBatch, *m_pWorldBoundingSphere, XMVectorSet(1.f, 0.f, 0.f, 1.f));break;
	default: return E_FAIL;
	}

	return S_OK;
}


HRESULT CBounding_Sphere::Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor)
{
	/* Draw */
	DX::Draw(pBatch, *m_pWorldBoundingSphere, XMLoadFloat4(&vColor));

	return S_OK;
}

#endif