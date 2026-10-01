#pragma once
#include "CComponent.h"
#include "CBounding_Sphere.h"
#include "CBounding_AABB.h"
#include "CBounding_OBB.h"

namespace Engine
{
	class CVIBuffer;
	class CShader;
}


NS_BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
protected:
		explicit CCollider(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
		explicit CCollider(const CCollider& Prototype);
		virtual ~CCollider() {}

protected:
	virtual HRESULT					Proto_Initialize()override;
	virtual HRESULT					Clone_Initialize(void* pArg)override;
	HRESULT							Load_ColliderData(const wstring& wstrFindKey);

protected:
	HRESULT							Ready_ColliderBuffer(CCollider* pCollider);
	virtual HRESULT					Set_GPU_ColliderLine() { return S_OK; }

public:
	virtual bool					Intersects(CCollider* pOther, CollisionInfo* pCollisionInfo);
	virtual bool					Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut);
	virtual void					Update_World(const Float4x4* pWorldMatrix);
	virtual void					Update_World(const XMatrix_1& pWorldMatrix);
	void							Reset_Collision();
public:
	void							Set_IsCollision(bool isCollision) { m_isCollision = isCollision; }
	void							Set_IsActive(bool isActive)		  { m_isActive    = isActive; }
public:
	COLLIDER_TYPE					Get_ColliderType()  const		  { return m_eColliderType;}
	bool							Get_IsCollision()   const		  { return m_isCollision;}
	bool							Get_IsActive()      const		  { return m_isActive;}
	CBounding*						Get_Bounding()		const		  { return m_pBounding;}
	Vector4							Get_ColliderWorldPos();

public:
	HRESULT							Render(FD CCameraObjectBase* pRenderCam);
public:
#ifdef _DEBUG
	virtual void					ImGuiUpdate();
	void							Save_Collider(const wstring& wstrSavePath);
	HRESULT							Change_Bounding(COLLIDER_TYPE eChangeType);			
	void							Set_Color(const Color& vColor) { m_vColor = vColor; }
	void							Set_IsTriggerBox(bool isTriggerBox) { m_isTriggerBox = isTriggerBox; }
#endif
protected:
	COLLIDER_TYPE					m_eColliderType;
	bool							m_isCollision;
	bool							m_isActive;
	CBounding*						m_pBounding;
#ifdef _DEBUG
	char										  m_szSaveColliderName[MAX_PATH];
	PrimitiveBatch<DirectX::VertexPositionColor>* m_pBatch;
	BasicEffect*								  m_pEffect;
	Color										  m_vColor;
	bool										  m_isTriggerBox;
#endif

public:
	static	CCollider*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CCollider*			Clone(void* pArg);
	virtual void        Free()override;
 };

NS_END

