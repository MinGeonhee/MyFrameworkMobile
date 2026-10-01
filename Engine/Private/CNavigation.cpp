#include "CNavigation.h"
#include "CCell.h"
#include "CGameManager.h"
#include "CShader.h"


CNavigation::CNavigation(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CComponent(pDevice, pContext), m_vecCell{ nullptr }, m_iOwnerCellIndex{}, m_pParentMatrix(nullptr), m_wstrKey{}
#ifdef _DEBUG
	, m_pShader(nullptr)
	,m_isRender(false)
#endif // _DEBUG
{
}

CNavigation::CNavigation(const CNavigation& Proto)
	: CComponent(Proto) , m_vecCell(Proto.m_vecCell), m_iOwnerCellIndex(Proto.m_iOwnerCellIndex), m_pParentMatrix(Proto.m_pParentMatrix)
	, m_wstrKey(Proto.m_wstrKey)
#ifdef _DEBUG
	, m_pShader(Proto.m_pShader) 
	,m_isRender(Proto.m_isRender)
#endif // _DEBUG


{
}

HRESULT CNavigation::Proto_Initialize()
{
	/* ProtoType 생성은 Cell만 들고있는다 */
	return S_OK;
}

HRESULT CNavigation::Clone_Initialize(void* pArg)
{
	NavigationCom_Desc* pDesc = static_cast<NavigationCom_Desc*>(pArg);

	m_pParentMatrix = pDesc->pParentMatrix;
	m_iOwnerCellIndex = pDesc->iCurMyCellIndex;
	m_wstrKey = pDesc->wstrNavigationKey;
	m_vecCell = CGameManager::GetInstance()->CellManager_Get_Cells(m_wstrKey);

#ifdef _DEBUG
	m_pShader = static_cast<CShader*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(L"Shader_Cell", RESOURCE_TYPE::SHADER, false));
#endif // _DEBUG
	return S_OK;
}


#ifdef _DEBUG
HRESULT	CNavigation::Render()
{
	//NULLPTR_RETURN_EFAIL(m_pShader);
	//if (m_isRender == true)
	//	return S_OK;

	//Float4x4 matWorld{};
	//XMStoreFloat4x4(&matWorld,XMMatrixIdentity());

	//m_pShader->Set_GPU_Transform(matWorld);

	//if(m_iOwnerCellIndex == FIELD_CELL_INDEX)
	//	for (auto& Cell : m_vecCell)
	//	{
	//		m_pShader->Apply_Pass("Default");

	//		Cell->Render();
	//	}
	//else
	//{
	//	m_pShader->Set_GPU_Vector(Float4(0.45f, 0.70f, 1.0f, 1.f), "g_CellColorVector");
	//	m_pShader->Apply_Pass("InCell");

	//	m_vecCell[m_iOwnerCellIndex]->Render();
	//}
	return S_OK;
}
#endif

HRESULT CNavigation::ChangeCells(const NavigationCom_Desc& tNavDesc)
{
	m_vecCell = CGameManager::GetInstance()->CellManager_Get_Cells(tNavDesc.wstrNavigationKey);
	if (m_vecCell.empty()) return E_FAIL;

	m_iOwnerCellIndex = tNavDesc.iCurMyCellIndex;
	m_pParentMatrix = tNavDesc.pParentMatrix;

	return S_OK;
}

Vector4 CNavigation::GetCellCenterPos(int32 iCellIndex)
{
	if (m_iOwnerCellIndex == -1) return XMVectorZero();

	return m_vecCell[iCellIndex]->Get_CenterPos();
}

Vector4 CNavigation::GetCurCellCenterPos()
{
	if (m_iOwnerCellIndex == -1) return XMVectorZero();
	 
	return m_vecCell[m_iOwnerCellIndex]->Get_CenterPos();
}

int32 CNavigation::ComputeCellIndex(const fVector4_1& vPos)
{

	if (m_vecCell.empty()) return -1;



	for (auto& Cell : m_vecCell)
	{
		if (!Cell) continue;

		if (Cell->Comput_IsInCell(vPos) == true)
			return Cell->Get_Index();

	}

	return -1;
}



CNavigation* CNavigation::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CNavigation* pNavigation = new CNavigation(pDevice, pContext);

	if (FAILED(pNavigation->Proto_Initialize()))
	{
		Safe_Release(pNavigation);

		MSG_BOX("Navigation is Failed to Clone");

		return nullptr;
	}
	return pNavigation;
}

CComponent* CNavigation::Clone(void* pArg)
{
	CNavigation* pNavigation = new CNavigation(*this);

	if (FAILED(pNavigation->Clone_Initialize(pArg)))
	{
		Safe_Release(pNavigation);

		MSG_BOX("Navigation is Failed to Clone");
	}
	return pNavigation;
}


HRESULT CNavigation::SetUp_NeighborIndex()
{
	for (auto& SourCell : m_vecCell)
		for (auto& DescCell : m_vecCell)
			SourCell->SetUp_NeighborCell(DescCell);

	return S_OK;
}

HRESULT CNavigation::Render(CCameraObjectBase* pRenderCam)
{
#ifdef _DEBUG

	NULLPTR_RETURN_EFAIL(m_pShader);
	if (m_isRender == true)
		return S_OK;

	Float4x4 matWorld{};
	XMStoreFloat4x4(&matWorld, XMMatrixIdentity());

	m_pShader->Set_GPU_Transform(matWorld);

	if (m_iOwnerCellIndex == FIELD_CELL_INDEX)
		for (auto& Cell : m_vecCell)
		{
			m_pShader->Apply_Pass("Default");

			Cell->Render();
		}
	else
	{
		m_pShader->Set_GPU_Vector(Float4(0.45f, 0.70f, 1.0f, 1.f), "g_CellColorVector");
		m_pShader->Apply_Pass("InCell");

		m_vecCell[m_iOwnerCellIndex]->Render();
	}
	return S_OK;


#endif // _DEBUG
	return S_OK;
}




Vector4 CNavigation::On_Plane(const fVector4_1& vPos)
{
	/* 원래 부모행렬 곱해서 그 Cell Space 공간으로 이동해야한다 하지만 나는 안곱해도돼 */
	return XMVectorSetY(vPos,m_vecCell[m_iOwnerCellIndex]->Compute_Height(vPos));
}

bool CNavigation::Is_Move(const fVector4_1& vResultPos)
{
	/* Field 라면 나가기 */
	if (m_iOwnerCellIndex == FIELD_CELL_INDEX )
		return false;

	/* vResult Pos => Go Straight 하고 나온 뒤의 좌표이다 */
	/* 이동이 가능한지 불가능한지 체크해주자...... */

	/* 삼각형을 나갔느지 안나갔느지 부터 체크해주자 */

	int32 iNeighborIndex = { -1 };

	/* False => 셀을 나간거다. */
	if (m_vecCell[m_iOwnerCellIndex]->IsIn(vResultPos, iNeighborIndex) == false)
	{
		/* 나갔던 삼각형의 이웃이 없다라면 */
		if (iNeighborIndex == No_Neighbor)
			return false;
		if (m_vecCell[iNeighborIndex]->Get_IsActive() == false)
			return false;


		else
		{
			while (true)
			{
				/* IsIn으로 정확한 검사 다시한번 */
				if (m_vecCell[iNeighborIndex]->IsIn(vResultPos, iNeighborIndex) == true)
					break;

				if (iNeighborIndex == No_Neighbor)
				{
					/* 여기들어오면 네이버가 없다는듰 */

					return false;
				}

			}

			if (m_vecCell[iNeighborIndex]->Get_IsActive() == false)
				return false;

			m_iOwnerCellIndex = iNeighborIndex;

			return true;
		}
	}

	return true;
}

bool CNavigation::Is_VailedPosition(const fVector4_1& vPos)
{
	/* Field 라면 나가기 */
	if (m_iOwnerCellIndex == FIELD_CELL_INDEX)
		return false;

	/* vResult Pos => Go Straight 하고 나온 뒤의 좌표이다 */
	/* 이동이 가능한지 불가능한지 체크해주자...... */

	/* 삼각형을 나갔느지 안나갔느지 부터 체크해주자 */

	int32 iNeighborIndex = { -1 };

	// [안전장치] 최대 50칸까지만 찾아보고, 그래도 안 나오면 포기 (무한루프 방지)
	int iSafetyCount = 0;
	const int MAX_SEARCH_COUNT = 50;

	/* False => 셀을 나간거다. */
	if (m_vecCell[m_iOwnerCellIndex]->IsIn(vPos, iNeighborIndex) == false)
	{
		/* 나갔던 삼각형의 이웃이 없다라면 */
		if (iNeighborIndex == No_Neighbor)
			return false;
		if (m_vecCell[iNeighborIndex]->Get_IsActive() == false)
			return false;


		else
		{
			while (iSafetyCount < MAX_SEARCH_COUNT)
			{
				iSafetyCount++;

				/* IsIn으로 정확한 검사 다시한번 */
				if (m_vecCell[iNeighborIndex]->IsIn(vPos, iNeighborIndex) == true)
					break;

				if (iNeighborIndex == No_Neighbor)
				{
					/* 여기들어오면 네이버가 없다는듰 */

					return false;
				}

			}

			if (m_vecCell[iNeighborIndex]->Get_IsActive() == false)
				return false;

			return true;
		}
	}

	return true;
}

Vector4 CNavigation::Is_MoveSliding(const fVector4_1& vLookDir, const fVector4_2& vCurPos, float fMoveDelta)
{
	/* Field라면 그냥 현재 내위치 Return */
	if (m_iOwnerCellIndex == FIELD_CELL_INDEX)
		return vCurPos;

	/* Field가 아니라면? */

	int32 iNeighborIndex = { -1 };

	Vector4 vFinalPos = vCurPos + XMVector3Normalize(vLookDir) * fMoveDelta;


	Float3 vOutNormal{};

	/* 여기서 Is In에다가 함수를어케넣어주지 */
	if (m_vecCell[m_iOwnerCellIndex]->IsIn(vFinalPos, &vOutNormal, iNeighborIndex ) == false)
	{
		if (iNeighborIndex == No_Neighbor || (m_vecCell[iNeighborIndex]->Get_IsActive() == false))
		{
			Vector4 vNormal = -XMLoadFloat3(&vOutNormal);
			Vector4 vSlide = vLookDir + vNormal * (Vector3Dot(-vLookDir, vNormal));

			float vAngle = XMConvertToDegrees(XMScalarACos(((Vector3Dot(vSlide,vLookDir)))));

			if (85.f <= vAngle && vAngle <= 95.f)
				return vCurPos;

			Vector4 vSlideFinalPos = vCurPos + XMVector3Normalize(vSlide) * fMoveDelta * 0.6f ;

			if (this->Is_Move(vSlideFinalPos))
				return vSlideFinalPos;
			else
				return vCurPos;
		}

		while (true)
		{
			/* IsIn으로 정확한 검사 다시한번 */
			if (m_vecCell[iNeighborIndex]->IsIn(vFinalPos, &vOutNormal, iNeighborIndex) == true)
				break;
			/* Neighbor가없을때 내 현재 위치값? */
			if (iNeighborIndex == No_Neighbor ||( m_vecCell[iNeighborIndex]->Get_IsActive() == false))
			{
				Vector4 vNormal = -XMLoadFloat3(&vOutNormal);
				Vector4 vSlide = vLookDir + vNormal * (Vector3Dot(-vLookDir, vNormal));

				float vAngle = XMConvertToDegrees(XMScalarACos(((Vector3Dot(vSlide, vLookDir)))));

				if (85.f <= vAngle && vAngle <= 95.f)
					return vCurPos;

				Vector4 vSlideFinalPos = vCurPos + XMVector3Normalize(vSlide) * fMoveDelta * 0.6f;

				if (this->Is_Move(vSlideFinalPos))
					return vSlideFinalPos;
				else
					return vCurPos;
			}
		}

		m_iOwnerCellIndex = iNeighborIndex;
		return vFinalPos;
	}

	return vFinalPos;
}

void CNavigation::Set_ParentMatrix(const Float4x4* pParentMatrix)
{
	NULLPTR_RETURN_NULL(pParentMatrix);

	m_pParentMatrix = pParentMatrix;
}

void CNavigation::Set_CellActive(int32 iCellIndex, bool isActive)
{
	if ((int32)m_vecCell.size() < iCellIndex)
		return;
	if (iCellIndex <= -1)
		return;
	
	m_vecCell[iCellIndex]->Set_IsActive(isActive);
}

void CNavigation::Free()
{
	m_vecCell.clear();

#ifdef _DEBUG
	Safe_Release(m_pShader);
#endif // _DEBUG

}
