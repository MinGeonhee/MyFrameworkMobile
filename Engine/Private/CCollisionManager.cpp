#include "CCollisionManager.h"
#include "CGameManager.h"
#include "CCollider.h"

CCollisionManager::CCollisionManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	:  m_pDevice(pDevice) , m_pDeviceContext(pContext), m_pGameManager(CGameManager::GetInstance())
	, m_vecCollisionTable{}, m_iMaxColliderGroupID{ 0 }, m_vecColliderGroup{}, m_vecPreCollisionInfo{} , m_vecCurCollisionInfo{}
{
	Safe_AddRef(m_pGameManager);
}

HRESULT CCollisionManager::Initialize(const CollisionManager_Desc& tDesc)
{
	m_iMaxColliderGroupID = tDesc.iMaxColliderGroupID;
	m_vecColliderGroup.resize(m_iMaxColliderGroupID);

	m_vecPreCollisionInfo.reserve(50);
	m_vecCurCollisionInfo.reserve(50);

	return S_OK;
}

HRESULT CCollisionManager::Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID)
{
	/* 중복테이블도 가능하게해야할듯 근데 중복테이블인경우 처리를 다르게 빼야 할것같음 */

	for (auto& Pair : m_vecCollisionTable)
	{
		if (Pair.first == iSrcGroupID && Pair.second == iDstGroupID) return E_FAIL;
		if (Pair.first == iDstGroupID && Pair.second == iSrcGroupID) return E_FAIL;
	}


	m_vecCollisionTable.push_back(pair<uint32, uint32>(iSrcGroupID, iDstGroupID));

	return S_OK;
}

HRESULT CCollisionManager::Add_Collider(uint32 iAddGroupID, CCollider* pCollider)
{
	NULLPTR_RETURN_EFAIL(pCollider);
	if (iAddGroupID > m_iMaxColliderGroupID) return E_FAIL;


	bool isSameCollider{ false };

	for (auto& Collider : m_vecColliderGroup[iAddGroupID])
	{
		/* 중복콜라이더 등록은 막아준다  */
		if (pCollider == Collider)
			isSameCollider = true;
	}

	if (isSameCollider == true)
		return S_OK;

	m_vecColliderGroup[iAddGroupID].push_back(pCollider);
	Safe_AddRef(pCollider);
	return S_OK;
}

void CCollisionManager::Clear_Collision()
{
	/* Collision Table Clear */
	m_vecCollisionTable.clear();
	vector<CollisionTable>().swap(m_vecCollisionTable);

	/* Collider Group Clear */
	for (auto& ColliderGroup : m_vecColliderGroup)
	{
		for (auto& Collider : ColliderGroup)
			Safe_Release(Collider);
		ColliderGroup.clear();
	}

	m_vecColliderGroup.resize(m_iMaxColliderGroupID);

}

void CCollisionManager::Clean_DeadCollision()
{
	for (auto& ColliderGroup : m_vecColliderGroup)
	{
		auto iter = std::remove_if(ColliderGroup.begin(), ColliderGroup.end(), [](CCollider* pCollider)->bool{
			CGameObject* pOwner = static_cast<CGameObject*>(pCollider->Get_Owner());
			if (pOwner->Get_Dead())
			{
				Safe_Release(pCollider);
				return true;
			}
			return false;
			});
		ColliderGroup.erase(iter,ColliderGroup.end());
	}


	auto iter2 = std::remove_if(m_vecPreCollisionInfo.begin(), m_vecPreCollisionInfo.end(), [](CollisionInfo tInfo) ->bool {
		if (tInfo.pSrcGameObject)
		{
			if (tInfo.pSrcGameObject->Get_Dead())
				return true;
		}
		if (tInfo.pDstGameObject)
		{
			if (tInfo.pDstGameObject->Get_Dead())
				return true;
		}

		return false;

		});

	m_vecPreCollisionInfo.erase(iter2,m_vecPreCollisionInfo.end());
}

void CCollisionManager::Reset_IsCollision()
{
	for (auto& ColliderGroup : m_vecColliderGroup)
		for (auto& Collider : ColliderGroup)
			Collider->Reset_Collision();
}

void CCollisionManager::RayCollision(uint32 iColliderGruopID,const RAY_INFO& tRayInfo , RayHit* pRayHit)
{
	for (auto& Collider : m_vecColliderGroup[iColliderGruopID])
	{
		if (!Collider) continue;
		if (Collider->Intersects_Ray(tRayInfo, pRayHit))
		{
			pRayHit->pRayHitGameObject->OnRayCollision(tRayInfo,pRayHit);

			if (pRayHit->isHit == true)
				return;
			else
				continue;
		}
	}
}

void CCollisionManager::Collision()
{
	static auto ProcessCollisionPair = [this](CCollider* pSrc, CCollider* pDst, const CollisionTable& TablePair) {

		NULLPTR_RETURN_NULL(pSrc); NULLPTR_RETURN_NULL(pDst);
		if (!pSrc->Get_IsActive() || !pDst->Get_IsActive()) return;

		CollisionInfo tCurCollisionInfo{};

		/* 이번 프레임에 충돌처리가 났을시 tCurCollisionInfo의 데이터가 채워진다 */
		if (!pSrc->Intersects(pDst, &tCurCollisionInfo))
			return;
		
		/* Table Index 정보 저장 */
		tCurCollisionInfo.iSrcGroupIndex = TablePair.first;
		tCurCollisionInfo.iDstGroupIndex = TablePair.second;
		
		bool isOnCollision = false;

		for (auto& tPreCollisionInfo : m_vecPreCollisionInfo)
		{
			/* Game Object 및 Collider 까지 검사 */
			if (tPreCollisionInfo.pSrcGameObject	== tCurCollisionInfo.pSrcGameObject
				&& tPreCollisionInfo.pDstGameObject == tCurCollisionInfo.pDstGameObject
				&& tPreCollisionInfo.pSrcCollider	== tCurCollisionInfo.pSrcCollider
				&& tPreCollisionInfo.pDstCollider	== tCurCollisionInfo.pDstCollider)
			{
				/* 이전 프레임에 충돌 처리였다면 현재 On Collision 상황 */
				tCurCollisionInfo.pSrcGameObject->OnCollision(tCurCollisionInfo);
				tCurCollisionInfo.pDstGameObject->OnCollision(tCurCollisionInfo.Swap());

				isOnCollision = true;
				break;
			}
		}
		

		if (!isOnCollision)
		{
			tCurCollisionInfo.pSrcGameObject->OnCollisionEnter(tCurCollisionInfo);
			tCurCollisionInfo.pDstGameObject->OnCollisionEnter(tCurCollisionInfo.Swap());
		}

		m_vecCurCollisionInfo.push_back(tCurCollisionInfo);

	};

	/* Collision Table에 등록된 그룹 순회 */
	for (auto& TablePair : m_vecCollisionTable)
	{
		ColliderGroup& SrcGroup = m_vecColliderGroup[TablePair.first];
		ColliderGroup& DstGroup = m_vecColliderGroup[TablePair.second];

		/* 같은테이블이라면 */
		if (TablePair.first == TablePair.second)
		{
			uint32 iSize = static_cast<uint32>(m_vecColliderGroup[TablePair.first].size());

			for (uint32 i = 0; i < iSize; ++i)
			{
				for (uint32 j = i+1 ; j < iSize; ++j)
				{
					ProcessCollisionPair(SrcGroup[i], DstGroup[j], TablePair);
				}
			}
		}
		/* 다른 테이블이라면 */
		else
		{
			for (auto& SrcCollider : SrcGroup)
			{
				for (auto& DstCollider : DstGroup)
				{
					ProcessCollisionPair(SrcCollider, DstCollider, TablePair);
				}
			}
		}
	}

	for(auto& PreCol : m_vecPreCollisionInfo)
	{
		bool isExit = true;

		for (auto& CurCol : m_vecCurCollisionInfo)
		{
			if (	PreCol.pSrcGameObject == CurCol.pSrcGameObject 
				&&	PreCol.pDstGameObject == CurCol.pDstGameObject
				&&	PreCol.pSrcCollider	  == CurCol.pSrcCollider
				&&	PreCol.pDstCollider	  == CurCol.pDstCollider)
			{
				isExit = false;
				break;;
			}
		}

		if (isExit)
		{
			PreCol.pSrcGameObject->OnCollisionExit(PreCol);
			PreCol.pDstGameObject->OnCollisionExit(PreCol.Swap());
		}
	}

	m_vecPreCollisionInfo = m_vecCurCollisionInfo;
	m_vecCurCollisionInfo.clear();
}



CCollisionManager* CCollisionManager::Create(const CollisionManager_Desc& tDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CCollisionManager* pCollisionManager = new CCollisionManager(pDevice, pContext);




	if (FAILED(pCollisionManager->Initialize(tDesc)))
	{
		Safe_Release(pCollisionManager);
		MSG_BOX(" Collision Manager is failed to Create ");
		return nullptr;
	}

	return pCollisionManager;
}

void CCollisionManager::Free()
{
	__super::Free();
	
	/* CCollisionManager::Free() */
	Safe_Release(m_pGameManager);

	for (auto& ColliderGroup : m_vecColliderGroup)
	{
		for (auto& Collider : ColliderGroup)
			Safe_Release(Collider);
		ColliderGroup.clear();
	}

	m_vecCollisionTable.clear();
	vector<CollisionTable>().swap(m_vecCollisionTable);
}
