#pragma once
#include "CBase.h"


namespace Engine
{
	class CGameManager;
	class CCollider;
}

NS_BEGIN(Engine)

/* uint32 , uint32 로 바꾼다고 치자 */
/* 내가 1,3 이랑 충돌처리가 들어갈 예정 */
/* 그렇게 되면 1,3 이랑 충돌테이블 조합식을 만들어줘 */
/* 이렇게 되면 1,3 이랑 충돌조합식을 만들어지고  */
/* m_umapColliderVec 여기서 들고오게될거요 , */


using ColliderGroup = vector<CCollider*>;

using CollisionTable = pair<uint32,uint32>;

class CCollisionManager : public CBase
{
private:
	explicit CCollisionManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CCollisionManager() {}
private:
	HRESULT Initialize(const CollisionManager_Desc& tDesc);
public:
	HRESULT	Register_CollisionTable(uint32 iSrcGroupID , uint32 iDstGroupID );
	HRESULT Add_Collider( uint32 iAddGroupID , CCollider* pCollider );
	void	Clear_Collision();
	void	Clean_DeadCollision();
	void	Reset_IsCollision();
public:
	void	RayCollision( uint32 iColliderGruopID  , const RAY_INFO& tRayInfo ,RayHit* pRayHit);
public:
	void	Collision();
private:
	uint32											m_iMaxColliderGroupID;
	vector< CollisionTable >						m_vecCollisionTable;
	vector<ColliderGroup>							m_vecColliderGroup;

	vector<CollisionInfo>							m_vecPreCollisionInfo;
	vector<CollisionInfo>							m_vecCurCollisionInfo;
private:
	ComPtr<Device>									m_pDevice;
	ComPtr<DeviceContext>							m_pDeviceContext;
	CGameManager*									m_pGameManager;
public:
	static CCollisionManager* Create(const CollisionManager_Desc& tDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END