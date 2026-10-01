#include "CCollider.h"
#include "CVIBuffer_ColliderBox.h"
#include "MyImGuiHeader.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CTransform.h"
#include "CFilePath.h"
#include "CFileFunction.h"
#include "CJsonFunction.h"
#include "CGameObject.h"
#include "EnumUtils.h"


CCollider::CCollider(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CComponent(pDevice, pContext), m_eColliderType(COLLIDER_TYPE::END)
	, m_isCollision(false), m_pBounding(nullptr), m_isActive(true)
#ifdef _DEBUG
	, m_pBatch(nullptr), m_pEffect(nullptr), m_szSaveColliderName{}, m_vColor{1.f,1.f ,1.f ,1.f }
	, m_isTriggerBox(false)
#endif // _DEBUG
{

}


CCollider::CCollider(const CCollider& Prototype)
	:	CComponent(Prototype),m_eColliderType(Prototype.m_eColliderType) 
	,  m_isCollision{Prototype.m_isCollision}, m_pBounding(Prototype.m_pBounding) , m_isActive(Prototype.m_isActive)
#ifdef _DEBUG
	, m_pBatch(Prototype.m_pBatch), m_pEffect(Prototype.m_pEffect), m_szSaveColliderName{}
	, m_vColor(Prototype.m_vColor)
	, m_isTriggerBox(Prototype.m_isTriggerBox)
#endif
{
#ifdef _DEBUG
#endif // _DEBUG

}

HRESULT CCollider::Proto_Initialize()
{

#ifdef _DEBUG
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pDeviceContext.Get());
	m_pEffect = new BasicEffect(m_pDevice.Get());
	m_pEffect->SetVertexColorEnabled(true);
#endif // _DEBUG

	return S_OK;
}

HRESULT CCollider::Clone_Initialize(void* pArg)
{
	if (FAILED(__super::Clone_Initialize(pArg)))
		return E_FAIL;

	COLLIDERCOM_DESC* pDesc = static_cast<COLLIDERCOM_DESC*>(pArg);

	if (!pDesc->wstrFindKeyName.empty())
	{
		if(FAILED(Load_ColliderData(pDesc->wstrFindKeyName)))
			return E_FAIL;
	}
	else
	{
		m_eColliderType = pDesc->eColliderType;

		switch (m_eColliderType)
		{
		case Engine::COLLIDER_TYPE::SPHERE:
			m_pBounding = CBounding_Sphere::Create(pArg,m_pDevice,m_pDeviceContext);
			break;
		case Engine::COLLIDER_TYPE::AABB:
			m_pBounding = CBounding_AABB::Create(pArg, m_pDevice, m_pDeviceContext);
			break;
		case Engine::COLLIDER_TYPE::OBB:
			m_pBounding = CBounding_OBB::Create(pArg, m_pDevice, m_pDeviceContext);
			break;
		default: break;
		}
	}

	NULLPTR_RETURN_EFAIL(m_pBounding);

	/* Buffer 생성 */

	return S_OK;
}

HRESULT CCollider::Ready_ColliderBuffer(CCollider* pCollider)
{
    CComponent* pCom  =	static_cast<CComponent*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype_InGlobal(
		PROTOTYPE_TYPE::COMPONENT , Proto_Com_VIBuffer_ColliderBox , nullptr));
	NULLPTR_RETURN_EFAIL(pCom);

	return S_OK;
}

bool CCollider::Intersects(CCollider* pOther,CollisionInfo* pCollisionInfo)
{
	if (pCollisionInfo == nullptr) return false;
	if (pOther == nullptr) return false;

	bool isCollision{ false };

	if(isCollision = m_pBounding->Intersects(pOther->Get_Bounding(), pCollisionInfo,pOther->m_eColliderType))
	{
		pCollisionInfo->pSrcGameObject = static_cast<CGameObject*>(this->m_pOwner);
		pCollisionInfo->pDstGameObject = static_cast<CGameObject*>(pOther->m_pOwner);

		pCollisionInfo->pSrcCollider = this;
		pCollisionInfo->pDstCollider = pOther;
		
		m_isCollision = true;
		pOther->m_isCollision = true;

	}
	else
	{
		pCollisionInfo->pSrcGameObject	= nullptr;
		pCollisionInfo->pDstGameObject	= nullptr;
		pCollisionInfo->pSrcCollider	= nullptr;
		pCollisionInfo->pDstCollider	= nullptr;
	}

	return isCollision;
}

bool CCollider::Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)
{
	if (m_pBounding->Intersects_Ray(tRayInfo, pRayOut))
	{
		pRayOut->pRayHitGameObject = static_cast<CGameObject*>(this->m_pOwner);
		pRayOut->SetHitPos(tRayInfo, pRayOut->fDistance);
		pRayOut->isHit = true;
	}
	else
		pRayOut->isHit = false;

	return pRayOut->isHit;
}

void CCollider::Update_World(const Float4x4* pWorldMatrix)
{
	m_pBounding->Update_World(pWorldMatrix);
}

void CCollider::Update_World(const XMatrix_1& WorldMatrix)
{
	m_pBounding->Update_World(WorldMatrix);
}

void CCollider::Reset_Collision()
{
	m_isCollision = false;
	return;
}

Vector4 CCollider::Get_ColliderWorldPos()
{
	return m_pBounding->Get_WorldPosition();
}

HRESULT	CCollider::Render(FD CCameraObjectBase* pRenderCam)
{

#ifdef _DEBUG


	auto* pMgr = CGameManager::GetInstance();


	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(pMgr->PipeLine_Get_PipeLineMatrixPtr(pRenderCam->GetName(), D3DTS::VIEW, false)));
	m_pEffect->SetProjection(XMLoadFloat4x4(pMgr->PipeLine_Get_PipeLineMatrixPtr(pRenderCam->GetName(), D3DTS::PROJ, false)));


	m_pEffect->Apply(m_pDeviceContext.Get());

	m_pBatch->Begin();

	if (m_isTriggerBox != true)
	{
		HRESULT hr{ E_FAIL };

		if (m_isCollision)
			hr = m_pBounding->Render(m_pBatch, COLLIDER_STATE::COLLISION);
		else if (m_isActive)
			hr = m_pBounding->Render(m_pBatch, COLLIDER_STATE::ACTIVE);
		else if (!m_isActive)
			hr = m_pBounding->Render(m_pBatch, COLLIDER_STATE::NON_ACTIVE);
		if (hr == E_FAIL)
			return E_FAIL;
		m_pBatch->End();
	}
	else
	{
		//if (!m_isActive) return S_OK;
		if (FAILED(m_pBounding->Render(m_pBatch, m_vColor)))
			return E_FAIL;

		m_pBatch->End();

		return S_OK;
	}

#endif // _DEBUG

	return S_OK;
}


CCollider* CCollider::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CCollider* pCollider = new CCollider(pDevice, pContext);

	if (FAILED(pCollider->Proto_Initialize()))
	{
		Safe_Release(pCollider);
		MSG_BOX("Collider is failed to Create");
		return nullptr;
	}

	return pCollider;
}

HRESULT CCollider::Load_ColliderData(const wstring& wstrFindKey)
{

	CFileStream fs(CFilePath::ColliderData_Path_Json);

	if (!fs.is_open()) return E_FAIL;

	nlohmann::ordered_json json;

	fs >> json;

	fs.close();

	string strName = WString_To_String(wstrFindKey);


	/* 이름관련 Check */
	if (!json.contains(strName))
	{
		/* 없다며면 구 콜라이더로 기본생성해주자 */
		CBounding_Sphere::BOUNDING_SPHERE_DESC pDesc{};
		m_eColliderType = COLLIDER_TYPE::SPHERE;
		pDesc.fRadius = 0.5f;
		pDesc.vLocalCenter = { 0.f ,0.f ,0.f };
		//MSG_BOX(" Can't Find Model Name Collider Data , Create Default Sphere Collider ...");
		m_pBounding = CBounding_Sphere::Create(&pDesc, m_pDevice, m_pDeviceContext);


#ifdef _DEBUG
		strcpy_s(m_szSaveColliderName, MAX_PATH, WString_To_String(wstrFindKey).c_str());
#endif // _DEBUG

	}
	else
	{
#ifdef _DEBUG
		strcpy_s(m_szSaveColliderName,MAX_PATH,WString_To_String(wstrFindKey).c_str());
#endif // _DEBUG


		auto& Collider_Data = json[strName];

		m_eColliderType = EnumUtils::COLLIDER_TYPE_String_To_Enum(Collider_Data["Type"].get<string>());

		switch (m_eColliderType)
		{
		case Engine::COLLIDER_TYPE::SPHERE:
		{
			CBounding_Sphere::BOUNDING_SPHERE_DESC tSphereDesc{};
			tSphereDesc.vLocalCenter.x = Collider_Data["Center"]["X"].get<float>();
			tSphereDesc.vLocalCenter.y = Collider_Data["Center"]["Y"].get<float>();
			tSphereDesc.vLocalCenter.z = Collider_Data["Center"]["Z"].get<float>();
			tSphereDesc.fRadius = Collider_Data["Radius"].get<float>();
			m_pBounding = CBounding_Sphere::Create(&tSphereDesc, m_pDevice, m_pDeviceContext);
		}
			break;
		case Engine::COLLIDER_TYPE::AABB:
		{
			CBounding_AABB::BOUNDING_AABB_DESC tAABBDesc{};
			tAABBDesc.vLocalCenter.x = Collider_Data["Center"]["X"].get<float>();
			tAABBDesc.vLocalCenter.y = Collider_Data["Center"]["Y"].get<float>();
			tAABBDesc.vLocalCenter.z = Collider_Data["Center"]["Z"].get<float>();

			tAABBDesc.vLocalExtents.x = Collider_Data["Extents"]["X"].get<float>();
			tAABBDesc.vLocalExtents.y = Collider_Data["Extents"]["Y"].get<float>();
			tAABBDesc.vLocalExtents.z = Collider_Data["Extents"]["Z"].get<float>();

			m_pBounding = CBounding_AABB::Create(&tAABBDesc, m_pDevice, m_pDeviceContext);
		}
			break;
		case Engine::COLLIDER_TYPE::OBB:
		{
			CBounding_OBB::BOUNDING_OBB_DESC tOBBDesc{};
			tOBBDesc.vLocalCenter.x = Collider_Data["Center"]["X"].get<float>();
			tOBBDesc.vLocalCenter.y = Collider_Data["Center"]["Y"].get<float>();
			tOBBDesc.vLocalCenter.z = Collider_Data["Center"]["Z"].get<float>();

			tOBBDesc.vLocalExtents.x = Collider_Data["Extents"]["X"].get<float>();
			tOBBDesc.vLocalExtents.y = Collider_Data["Extents"]["Y"].get<float>();
			tOBBDesc.vLocalExtents.z = Collider_Data["Extents"]["Z"].get<float>();

			tOBBDesc.vLocalRotation.x = Collider_Data["Rotation"]["X"].get<float>();
			tOBBDesc.vLocalRotation.y = Collider_Data["Rotation"]["Y"].get<float>();
			tOBBDesc.vLocalRotation.z = Collider_Data["Rotation"]["Z"].get<float>();

			m_pBounding = CBounding_OBB::Create(&tOBBDesc, m_pDevice, m_pDeviceContext);
		}
			break;
		default:
			break;
		}
	}

	NULLPTR_RETURN_EFAIL(m_pBounding);

	return S_OK;
}

CCollider* CCollider::Clone(void* pArg)
{
	CCollider* pCollider = new CCollider(*this);

	if (FAILED(pCollider->Clone_Initialize(pArg)))
	{
		Safe_Release(pCollider);
		MSG_BOX(" Collider Clone is failed to Create ");
		return nullptr;
	}

	return pCollider;

}

void CCollider::Free()
{
	__super::Free();

	/* CCollider::Free() */

#ifdef  _DEBUG

	if (m_eMakeType == MAKE_TYPE::PROTOTYPE)
	{
		/* 나는 클론마다 Effect를 생성해주고잇으니까..이럼어카지 Effect계속Set해줘야하는데..흠 */
		Safe_Delete(m_pEffect);
		Safe_Delete(m_pBatch);
	}

#endif 

	Safe_Release(m_pBounding);

}



#ifdef _DEBUG
void CCollider::ImGuiUpdate()
{
	ImGui::Text("--- Data Save ---");

	ImGui::InputText(" Save Name ", m_szSaveColliderName, MAX_PATH);

	if (ImGui::Button("Save", ImVec2(48, 24)))
	{
		this->Save_Collider(CFilePath::ColliderData_Path_Json);
	}

	ImGui::Checkbox(" Active ", &m_isActive);

	ImGui::Text("--- Change Type ---");

	ImGui::Text("Current Type => [ %s ] " , EnumUtils::COLLIDER_TYPE_Enum_To_String(m_eColliderType).c_str());

	if (ImGui::Button(" Sphere ", ImVec2(48, 24)))	{ this->Change_Bounding(COLLIDER_TYPE::SPHERE); }
	ImGui::SameLine();
	if (ImGui::Button(" AABB ", ImVec2(48, 24)))	{ this->Change_Bounding(COLLIDER_TYPE::AABB); }
	ImGui::SameLine();
	if (ImGui::Button(" OBB ", ImVec2(48, 24)))		{ this->Change_Bounding(COLLIDER_TYPE::OBB);}

	if (ImGui::TreeNode("--- Bounding Info ---"))
	{
		if (m_pBounding->ImGuiUpdate())
		{
			static_cast<CGameObject*>(m_pOwner)->Collider_Update(static_cast<CGameObject*>(m_pOwner)->GetTransform()->Get_matWorldPtr());
		}
		ImGui::TreePop();
	}
}

void CCollider::Save_Collider(const wstring& wstrSavePath)
{
	if (m_szSaveColliderName[0] == '\0') return;

	CFileStream fs(wstrSavePath);
	nlohmann::ordered_json Json;
	if (fs.is_open())
	{
		fs >> Json;
		fs.close();
	}
	/* 파일 열기 */
	ofstream os(wstrSavePath);

	/* Collider Type Save */


	auto& SaveJson = Json[m_szSaveColliderName];
	SaveJson["Type"] = EnumUtils::COLLIDER_TYPE_Enum_To_String(m_eColliderType);
	m_pBounding->Save_Bounding(SaveJson);

	os << Json;
}

HRESULT CCollider::Change_Bounding(COLLIDER_TYPE eChangeType)
{
	/* 같은 타입이면 Return */
	if (m_eColliderType == eChangeType) return E_FAIL;

	m_eColliderType = eChangeType;

	/* Bounding Box Reelase */
	Safe_Release(m_pBounding);

	switch (m_eColliderType)
	{
	case Engine::COLLIDER_TYPE::SPHERE:
	{
		CBounding_Sphere::BOUNDING_SPHERE_DESC tSphereDesc{};
		m_pBounding = CBounding_Sphere::Create(&tSphereDesc , m_pDevice, m_pDeviceContext);
		break;
	}
	case Engine::COLLIDER_TYPE::AABB:
	{
		CBounding_AABB::BOUNDING_AABB_DESC tAABBDesc{};
		m_pBounding = CBounding_AABB::Create(&tAABBDesc, m_pDevice, m_pDeviceContext);
		break;
	}
		break;
	case Engine::COLLIDER_TYPE::OBB:
	{
		CBounding_OBB::BOUNDING_OBB_DESC tOBBDesc{};
		m_pBounding = CBounding_OBB::Create(&tOBBDesc, m_pDevice, m_pDeviceContext);
		break;
	}
	default:
		break;
	}

	NULLPTR_RETURN_EFAIL(m_pBounding);

	return S_OK;
}



#endif 
