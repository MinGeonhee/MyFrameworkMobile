#pragma once
#include "CBase.h"

/* GameObject Clone을 만들어줄 클론객체 */
/* Clone Initialize , Proto Initialize 나누기  */
NS_BEGIN(Engine)

class CGameManager;
class CRenderManager;
class CComponent;
class CTransform;
class CCameraObjectBase;
class CCollider;



class ENGINE_DLL CGameObject abstract :public CBase
{
protected:
	explicit								CGameObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit								CGameObject(const CGameObject& pGameObject);
	virtual									~CGameObject();
protected:
	HRESULT									Proto_Initialize();				// Proto 생성시 불릴 초기화 
	virtual HRESULT							Clone_Initialize(void* pArg); // Clone 생성시 불릴 초기화
protected:
	virtual HRESULT							Ready_Component(void* pArg);
	HRESULT									Add_Component(uint32 iFindSceneID, CONST std::wstring& wstrFindProtoName, CONST std::wstring& wstrAddName,void* pArg , FD CComponent** ppOut);
public:
	virtual void							Pre_Update(float DT);
	virtual void							Update(float DT);
	virtual void							Late_Update(float DT);
	virtual HRESULT							Add_RenderGroup();
	virtual HRESULT							Render(FD CCameraObjectBase* pRenderCam);
	virtual void							Semi_Update();
public:
	void									Set_AllColliderActive(bool isActive);
public:

	/* 충돌 Event 처리관련 */
	virtual void							OnCollisionEnter(const CollisionInfo& tInfo) {};
	virtual void							OnCollision(const CollisionInfo& tInfo)		 {};
	virtual void							OnCollisionExit(const CollisionInfo& tInfo)  {};
	virtual void							OnRayCollision(const RAY_INFO& tRayInfo,RAY_HIT* pRayHit)			 {};

public:
	virtual void							Add_ColliderGroup()	PURE;
public:
#ifdef _DEBUG
	virtual void							ImGuiUpdate();
	HRESULT									Debug_ColliderAddRender();
#endif

public:
	void									Collider_Update(const Float4x4* matWorld);
public:
	/* Get */
	const map<const wstring, CComponent*>&	GetComponentList() const{ return m_mapComponent; }
	bool									Get_Dead()					const { return m_isDead; }
	const wstring&							GetName()					const { return m_strName; }
	CTransform*								GetTransform()				const{ return m_pTransformCom; }
	const float								Get_Z()						const;
	virtual wstring							Get_ModelName()				const { return m_strName; }

	bool									Get_IsRenderCulling() const { return m_isRenderCulling; }

	int32									Get_EventIndex()			const { return m_iEventIndex; }
	uint32									Get_Index()					const { return m_iIndex; }
	const bool&								Get_IsExptectedDelete()		const { return m_isExpectedDelete; }
	CGameObject*							Get_TargetObject() { return m_pTargetObject; }
	bool									Get_IsUpdatePhaseActive(UPDATE_PHASE eUpdatePhase) { return m_isUpdatePahseActive[Cast_Uint(eUpdatePhase)]; }
	CCollider*								Get_Collider(uint32 iIndex = 0)		const { return m_vecColliderCom[iIndex]; }
	const vector<CCollider*>&				Get_Colliders()						const { return m_vecColliderCom; }
	bool									Get_IsColliderActive(uint32 iIndex = 0 )      const;
	CGameObject*							Get_CollisionObject()				const{ return m_pCollisionObject;}
	int32									Get_MainType()						const { return m_iMainType; }
	bool									Get_IsPartObject()					const { return m_isPartObject; }
	virtual bool							Get_IsInCamArea(const Float4& vRenderCamAreaXZ);
public:
	/* Set */
	virtual void							Set_Dead();
	virtual void							Respawn(const Float4& vRespawnPos);
	
	void									Set_IsPartObject(bool isPart) { m_isPartObject = isPart; }
	void									Set_MainType(int32 iType) { m_iMainType = iType; }
	void									Set_CollisionObject(CGameObject* pCollisionObject) { m_pCollisionObject = pCollisionObject;}
	void									Set_IsDead(bool isDead) { m_isDead = isDead; }
	void									Set_IsRenderCulling(bool isCulling) {m_isRenderCulling = isCulling;}
	void									Set_EventIndex(int32 iEventIndex) { m_iEventIndex = iEventIndex; }
	void									Set_ColliderActive(bool isActive,uint32 iIndex = 0);
	void									SetName(const wstring& wstrName)	{ m_strName = wstrName; }
	void									Set_Index(uint32 iIndex)			{ m_iIndex = iIndex; }
	void									Set_IsExpectedDelete(bool isDelete) { m_isExpectedDelete = isDelete; }

	void									Set_UpdatePhaseActive(UPDATE_PHASE eUpdatePhase, bool isActive) { m_isUpdatePahseActive[Cast_Uint(eUpdatePhase)] = isActive; }
	void									Set_UpdatePhaseAllActive(bool isActive) 
	{
		for (int i = 0; i < Cast_Uint(UPDATE_PHASE::END); ++i)
			m_isUpdatePahseActive[i] = isActive;
	}
	void									Set_UpdatePahseActive(bool* isUpdatePhaseActive)
	{
		for (int i = 0; i < Cast_Uint(UPDATE_PHASE::END); ++i)
			m_isUpdatePahseActive[i] = isUpdatePhaseActive[i];
	}

	virtual void							Set_TargetObject(CGameObject* pGameObject) { m_pTargetObject = pGameObject;}
protected:
	CGameObject*							m_pTargetObject;
	CGameObject*							m_pCollisionObject;
protected:
	int32									m_iMainType;
	bool									m_isPartObject;
	uint32									m_iIndex;
	int32									m_iEventIndex;
protected:
	wstring									m_strName;
	bool									m_isDead;
	bool									m_isUpdatePahseActive[ Cast_Uint(UPDATE_PHASE::END) ];
	bool									m_isRenderCulling;
protected:
	map<const wstring, CComponent* >		m_mapComponent;
	CTransform*								m_pTransformCom;
	vector<CCollider*>						m_vecColliderCom;
protected:
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
	CGameManager*							m_pGameManager;
public:
	bool									m_isExpectedDelete;
	MAKE_TYPE								m_eMakeType;
public:
	virtual CGameObject*	Clone(void* pArg) PURE;
	virtual	void			Free() override;
};

NS_END

