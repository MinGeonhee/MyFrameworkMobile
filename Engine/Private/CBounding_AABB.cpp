#include "CBounding_AABB.h"
#include "CBounding_Sphere.h"
#include "CBounding_OBB.h"
#include "DebugDraw.h"

CBounding_AABB::CBounding_AABB(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CBounding(pDevice,pContext) , m_pLocalBoundingBox(nullptr) , m_pWorldBoundingBox(nullptr)
{
}

HRESULT CBounding_AABB::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	CBounding_AABB::BOUNDING_AABB_DESC* pDesc = static_cast<CBounding_AABB::BOUNDING_AABB_DESC*>(pArg);

	m_pLocalBoundingBox = new BoundingBox(pDesc->vLocalCenter, pDesc->vLocalExtents);
	NULLPTR_RETURN_EFAIL(m_pLocalBoundingBox);
	m_pWorldBoundingBox = new BoundingBox(*m_pLocalBoundingBox);
	NULLPTR_RETURN_EFAIL(m_pWorldBoundingBox);

	return S_OK;
}

void CBounding_AABB::Update_World(const Float4x4* pWorldMatrix)
{
	/* */
	Float3 vScale{
		sqrtf(pWorldMatrix->_11* pWorldMatrix->_11 + pWorldMatrix->_12 * pWorldMatrix->_12 + pWorldMatrix->_13 * pWorldMatrix->_13),
		sqrtf(pWorldMatrix->_21 * pWorldMatrix->_21 + pWorldMatrix->_22 * pWorldMatrix->_22 + pWorldMatrix->_23 * pWorldMatrix->_23),
		sqrtf(pWorldMatrix->_31 * pWorldMatrix->_31 + pWorldMatrix->_32 * pWorldMatrix->_32 + pWorldMatrix->_33 * pWorldMatrix->_33)
	};

	m_pWorldBoundingBox->Center =
	{
		pWorldMatrix->_41 + m_pLocalBoundingBox->Center.x * vScale.x,
		pWorldMatrix->_42 + m_pLocalBoundingBox->Center.y * vScale.y,
		pWorldMatrix->_43 + m_pLocalBoundingBox->Center.z * vScale.z
	};

	m_pWorldBoundingBox->Extents =
	{
		m_pLocalBoundingBox->Extents.x * fabsf(vScale.x),
		m_pLocalBoundingBox->Extents.y * fabsf(vScale.y),
		m_pLocalBoundingBox->Extents.z * fabsf(vScale.z)
	};
}

void CBounding_AABB::Update_World(const XMatrix_1& WorldMatrix)
{
	XMVECTOR r0 = WorldMatrix.r[0];
	XMVECTOR r1 = WorldMatrix.r[1];
	XMVECTOR r2 = WorldMatrix.r[2];
	XMVECTOR r3 = WorldMatrix.r[3];

	Float3 vScale{
		sqrtf(XMVectorGetX(r0) * XMVectorGetX(r0) + XMVectorGetY(r0) * XMVectorGetY(r0) + XMVectorGetZ(r0) * XMVectorGetZ(r0)),
		sqrtf(XMVectorGetX(r1) * XMVectorGetX(r1) + XMVectorGetY(r1) * XMVectorGetY(r1) + XMVectorGetZ(r1) * XMVectorGetZ(r1)),
		sqrtf(XMVectorGetX(r2) * XMVectorGetX(r2) + XMVectorGetY(r2) * XMVectorGetY(r2) + XMVectorGetZ(r2) * XMVectorGetZ(r2))
	};

	m_pWorldBoundingBox->Center =
	{
		XMVectorGetX(r3) + m_pLocalBoundingBox->Center.x * vScale.x,
		XMVectorGetY(r3) + m_pLocalBoundingBox->Center.y * vScale.y,
		XMVectorGetZ(r3) + m_pLocalBoundingBox->Center.z * vScale.z
	};

	m_pWorldBoundingBox->Extents =
	{
		m_pLocalBoundingBox->Extents.x * fabsf(vScale.x),
		m_pLocalBoundingBox->Extents.y * fabsf(vScale.y),
		m_pLocalBoundingBox->Extents.z * fabsf(vScale.z)
	};
}

Vector4 CBounding_AABB::Get_WorldPosition()
{
	return XMVectorSetW(XMLoadFloat3(&m_pWorldBoundingBox->Center), 1.f);
}

bool CBounding_AABB::Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)
{
	return m_pWorldBoundingBox->Intersects(XMLoadFloat3(&tRayInfo.vRayOrigin), XMLoadFloat3(&tRayInfo.vRayDir), pRayOut->fDistance);;
}

bool CBounding_AABB::Intersects(CBounding* pOtherBounding, COLLISION_INFO* pOutInfo , COLLIDER_TYPE eType)
{
	if (pOtherBounding == nullptr) return false;

	switch (eType)
	{
	case COLLIDER_TYPE::SPHERE: return MyIntersect_Sphere(static_cast<CONST CBounding_Sphere*>(pOtherBounding)->Get_WorldBoundingSphere(),pOutInfo);
		break;
	case COLLIDER_TYPE::AABB:	return MyIntersect_AABB(static_cast<CONST CBounding_AABB*>(pOtherBounding)->Get_WorldBoundingBox(),pOutInfo);
		break;
	case COLLIDER_TYPE::OBB:	return m_pWorldBoundingBox->Intersects(*static_cast<CONST CBounding_OBB*>(pOtherBounding)->Get_WorldBoundingOrientedBox());
		break;
	default:
		break;
	}

	return false;
}

bool CBounding_AABB::MyIntersect_AABB(const BoundingBox* pOtherBounding, COLLISION_INFO* pOutInfo)
{
	/* AABB 와 AABB끼리의 충돌 */
	XMVECTOR CenterA = XMLoadFloat3(&m_pWorldBoundingBox->Center);
	XMVECTOR ExtentsA = XMLoadFloat3(&m_pWorldBoundingBox->Extents);

	XMVECTOR CenterB = XMLoadFloat3(&pOtherBounding->Center);
	XMVECTOR ExtentsB = XMLoadFloat3(&pOtherBounding->Extents);

	XMVECTOR MinA = XMVectorSubtract(CenterA, ExtentsA);
	XMVECTOR MaxA = XMVectorAdd(CenterA, ExtentsA);

	XMVECTOR MinB = XMVectorSubtract(CenterB, ExtentsB);
	XMVECTOR MaxB = XMVectorAdd(CenterB, ExtentsB);

	/* 이게 충돌했다 안했다... */
	if (XMVectorGetX(MinA) > XMVectorGetX(MaxB)) return false;
	if (XMVectorGetX(MaxA) < XMVectorGetX(MinB)) return false;

	if (XMVectorGetY(MinA) > XMVectorGetY(MaxB)) return false;
	if (XMVectorGetY(MaxA) < XMVectorGetY(MinB)) return false;

	if (XMVectorGetZ(MinA) > XMVectorGetZ(MaxB)) return false;
	if (XMVectorGetZ(MaxA) < XMVectorGetZ(MinB)) return false;


	/* 충돌이 일어났다면? */

	float OverLapX = min(XMVectorGetX(MaxB), XMVectorGetX(MaxA)) - max(XMVectorGetX(MinB), XMVectorGetX(MinA));
	float OverLapY = min(XMVectorGetY(MaxB), XMVectorGetY(MaxA)) - max(XMVectorGetY(MinB), XMVectorGetY(MinA));
	float OverLapZ = min(XMVectorGetZ(MaxB), XMVectorGetZ(MaxA)) - max(XMVectorGetZ(MinB), XMVectorGetZ(MinA));
	
	if (OverLapX < OverLapY)
	{
		if (OverLapX < OverLapZ)
		{
			/* X < (Z , Y) ? */
			pOutInfo->fDepth = OverLapX;
			if (XMVectorGetX(CenterA) > XMVectorGetX(CenterB))
			{
				pOutInfo->vSrcPenetrationDir = { +1.f,0.f,0.f };
				pOutInfo->vDstPenetrationDir = { -1.f,0.f,0.f };
			}
			else
			{
				pOutInfo->vSrcPenetrationDir = { -1.f,0.f,0.f };
				pOutInfo->vDstPenetrationDir = { +1.f,0.f,0.f };
			}
		}
		else
		{
			/* Z <  X < Y */
			pOutInfo->fDepth = OverLapZ;
			if (XMVectorGetZ(CenterA) > XMVectorGetZ(CenterB))
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,0.f,+1.f };
				pOutInfo->vDstPenetrationDir = { 0.f,0.f,-1.f };
			}
			else
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,0.f,-1.f };
				pOutInfo->vDstPenetrationDir = { 0.f,0.f,+1.f };
			}
		}
	}
	else
	{
		if (OverLapY < OverLapZ)
		{
			/* Y <  (Z , X)? */
			pOutInfo->fDepth = OverLapY;
			if (XMVectorGetY(CenterA) > XMVectorGetY(CenterB))
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,+1.f,0.f };
				pOutInfo->vDstPenetrationDir = { 0.f,-1.f,0.f };
			}
			else
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,-1.f,0.f };
				pOutInfo->vDstPenetrationDir = { 0.f,+1.f,0.f };
			}
		}
		else
		{
			/* Z <  Y < X */
			pOutInfo->fDepth = OverLapZ;
			if (XMVectorGetZ(CenterA) > XMVectorGetZ(CenterB))
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,0.f,+1.f };
				pOutInfo->vDstPenetrationDir = { 0.f,0.f,-1.f };
			}
			else
			{
				pOutInfo->vSrcPenetrationDir = { 0.f,0.f,-1.f };
				pOutInfo->vDstPenetrationDir = { 0.f,0.f,+1.f };
			}
		}
	}

	return true;
}

bool CBounding_AABB::MyIntersect_Sphere(const BoundingSphere* pOtherBounding, COLLISION_INFO* pOutInfo)
{
	XMVECTOR SphereCenter = XMLoadFloat3(&pOtherBounding->Center);
	XMVECTOR SphereRadius = XMVectorReplicatePtr(&pOtherBounding->Radius);

	XMVECTOR BoxCenter = XMLoadFloat3(&m_pWorldBoundingBox->Center);
	XMVECTOR BoxExtents = XMLoadFloat3(&m_pWorldBoundingBox->Extents);

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

		XMStoreFloat3(&pOutInfo->vSrcPenetrationDir, -vNormalDir);
		XMStoreFloat3(&pOutInfo->vDstPenetrationDir, +vNormalDir);
		pOutInfo->fDepth = XMVectorGetX(SphereRadius) - fLength;
		return true;
	}
	else
		return false;

	return false;
}


CBounding_AABB* CBounding_AABB::Create(void* pArg, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CBounding_AABB* pBounding = new CBounding_AABB(pDevice, pContext);
	if (FAILED(pBounding->Initialize(pArg)))
	{
		Safe_Release(pBounding);
		MSG_BOX("AABB Create failed to Create");
		return nullptr;
	}

	return pBounding;
}

void CBounding_AABB::Free()
{
	__super::Free();

	Safe_Delete(m_pLocalBoundingBox);
	Safe_Delete(m_pWorldBoundingBox);
}



#ifdef _DEBUG
bool CBounding_AABB::ImGuiUpdate()
{
	bool isAction{ false };
	/* Center */
	ImGui::Text("--- Center --- ");
	if (ImGui::InputFloat("X##1", &m_pLocalBoundingBox->Center.x, 0.1f)) {isAction = true;}
	if(ImGui::InputFloat("Y##1",&m_pLocalBoundingBox->Center.y,0.1f))	 {isAction = true;}
	if(ImGui::InputFloat("Z##1",&m_pLocalBoundingBox->Center.z,0.1f))    {isAction = true;}


	/* Extents */
	ImGui::Text("--- Extents --- ");
	if(ImGui::InputFloat("X##2", &m_pLocalBoundingBox->Extents.x, 0.1f))	{isAction = true;}
	if(ImGui::InputFloat("Y##2", &m_pLocalBoundingBox->Extents.y, 0.1f))	{isAction = true;}
	if (ImGui::InputFloat("Z##2", &m_pLocalBoundingBox->Extents.z, 0.1f))	{isAction = true;}

	return isAction;
}

void CBounding_AABB::Save_Bounding(nlohmann::ordered_json& json)
{
	json["Center"]  = { { "X" , m_pLocalBoundingBox->Center.x}  , {"Y", m_pLocalBoundingBox->Center.y}  , {"Z", m_pLocalBoundingBox->Center.z} };
	json["Extents"] = { { "X" , m_pLocalBoundingBox->Extents.x} , {"Y", m_pLocalBoundingBox->Extents.y} , {"Z", m_pLocalBoundingBox->Extents.z} };
}
HRESULT CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)
{
	switch (eColliderState)
	{
	case Engine::COLLIDER_STATE::ACTIVE:		DX::Draw(pBatch, *m_pWorldBoundingBox, XMVectorSet(0.f, 0.f, 1.f, 1.f)); break;
	case Engine::COLLIDER_STATE::NON_ACTIVE:	DX::Draw(pBatch, *m_pWorldBoundingBox, XMVectorSet(0.f, 1.f, 0.f, 1.f));break;
	case Engine::COLLIDER_STATE::COLLISION:		DX::Draw(pBatch, *m_pWorldBoundingBox, XMVectorSet(1.f, 0.f, 0.f, 1.f));break;
	default: return E_FAIL;
	}

	return S_OK;
}

HRESULT CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor)
{
	DX::Draw(pBatch, *m_pWorldBoundingBox , XMLoadFloat4(&vColor));
	return S_OK;
}
#endif