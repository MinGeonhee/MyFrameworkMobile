#include "CPrototypeManager.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "CNavigation.h"
#include "CCollider.h"
#include "CVIBuffer.h"
#include "CUI_Image.h"
#include "CUITransform.h"
#include "CUI_Animation.h"
#include "CVIBuffer_Rect.h"



CPrototypeManager::CPrototypeManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext) ,  m_iTotalSceneNum(0)
{
	for (auto i = 0; i < Cast_Int(PROTOTYPE_TYPE::END); ++i)
		vector<map<const wstring, CBase*>>().swap(m_mapPrototype[i]);
}

HRESULT CPrototypeManager::Initialize(CONST PrototypeManager_Desc& tDesc)
{

	m_iTotalSceneNum = tDesc.iTotalSceneNum;

	for (auto i = 0; i < Cast_Int(PROTOTYPE_TYPE::END); ++i)
	{
		vector<map<const wstring, CBase*>>().swap(m_mapPrototype[i]);
		m_mapPrototype[i].resize(m_iTotalSceneNum);
	}

	return S_OK;
}

HRESULT CPrototypeManager::Ready_DefaultPrototype(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{

	CComponent* pProtoComponent{ nullptr };

#pragma region Component

	/* GameObject Compoent */
	pProtoComponent = CTransform::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_Component_Transform", pProtoComponent);

	pProtoComponent = CCollider::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_Component_Collider", pProtoComponent);

	pProtoComponent = CNavigation::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_Component_Navigation", pProtoComponent);

	/* UIObject Compoent */
	pProtoComponent = CUITransform::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_UIComponent_UITransform", pProtoComponent);

	pProtoComponent = CUI_Image::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_UIComponent_UIImage", pProtoComponent);

	pProtoComponent = CUI_Animation::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pProtoComponent);
	Add_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, L"Proto_UIComponent_UIAnimation", pProtoComponent);


#pragma endregion


	return S_OK;
}



HRESULT CPrototypeManager::Add_Prototype(PROTOTYPE_TYPE eAddProtoType, uint32 iAddProtoSceneID, const wstring& strAddProtoKey, CBase* pPrototype)
{
	const auto& pBase = Find_Prototype(eAddProtoType, iAddProtoSceneID, strAddProtoKey);
	/* ProtoType 이 찾아서 있다면 Return 없을 떄만 추가 */
	if (pBase != nullptr) return E_FAIL;
	/* 추가해주기 */
	m_mapPrototype[static_cast<int>(eAddProtoType)][iAddProtoSceneID].emplace(strAddProtoKey,pPrototype);

	return S_OK;
}

HRESULT CPrototypeManager::Add_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType, const wstring& strAddProtoKey, CBase* pPrototype)
{
	const auto& pBase = Find_Prototype_InGlobal(eProtoType,strAddProtoKey);
	/* ProtoType 이 찾아서 있다면 Return 없을 떄만 추가 */
	if (pBase != nullptr) return E_FAIL;
	/* 추가해주기 */
	m_mapPrototypeGlobal[Cast_Uint(eProtoType)].emplace(strAddProtoKey, pPrototype);

	return S_OK;
}

CBase* CPrototypeManager::Clone_Prototype(PROTOTYPE_TYPE eCloneProtoType, uint32 iCloneProtoSceneID, const wstring& strCloneProtoKey, void* pArg)
{
	const auto& pBase = Find_Prototype(eCloneProtoType, iCloneProtoSceneID, strCloneProtoKey);
	NULLPTR_RETURN_NULLPTR(pBase);

	if (eCloneProtoType == PROTOTYPE_TYPE::GAMEOBJECT) return static_cast<CGameObject*>(pBase)->Clone(pArg);
	if(eCloneProtoType == PROTOTYPE_TYPE::COMPONENT) return static_cast<CComponent*>(pBase)->Clone(pArg);

	return nullptr;
}

CBase* CPrototypeManager::Clone_Prototype_InGlobal(PROTOTYPE_TYPE eCloneProtoType, const wstring& strCloneProtoKey, void* pArg)
{
	CBase* pBase = Find_Prototype_InGlobal(eCloneProtoType,strCloneProtoKey);

	NULLPTR_RETURN_NULLPTR(pBase);


	if (eCloneProtoType == PROTOTYPE_TYPE::GAMEOBJECT) return static_cast<CGameObject*>(pBase)->Clone(pArg);
	if (eCloneProtoType == PROTOTYPE_TYPE::COMPONENT) return static_cast<CComponent*>(pBase)->Clone(pArg);

	return nullptr;
}

void CPrototypeManager::Clear_Prototype_CurSceneID(uint32 iCurSceneID)
{
	for (auto i = 0; i < (uint32)PROTOTYPE_TYPE::END; ++i)
	{
		for (auto& ProtoVec : m_mapPrototype[i][iCurSceneID])
			Safe_Release(ProtoVec.second);
		m_mapPrototype[i][iCurSceneID].clear();
	}
}

map<const wstring, CBase*>& CPrototypeManager::GetGameObjectPrototypeList_ByScene(uint32 iSceneID)
{
	return m_mapPrototype[(uint32)PROTOTYPE_TYPE::GAMEOBJECT][iSceneID];
}

map<const wstring, CBase*>& CPrototypeManager::GetComponentPrototypeList_ByScene(uint32 iSceneID)
{
	return m_mapPrototype[(uint32)PROTOTYPE_TYPE::COMPONENT][iSceneID];
}

CBase* CPrototypeManager::Find_Prototype(PROTOTYPE_TYPE eProtoType, uint32 iProtoSceneID, const wstring& strProtoKey)
{
	MAX_ENUM_CHECK_RETURN_NULLPTR( (uint32)PROTOTYPE_TYPE::END , eProtoType);
	MAX_ENUM_CHECK_RETURN_NULLPTR(m_iTotalSceneNum, iProtoSceneID);

	const auto& MapBase = m_mapPrototype[Cast_Uint(eProtoType)][iProtoSceneID];

	const auto& iter = MapBase.find(strProtoKey);

	if (iter == MapBase.end()) return nullptr;


	return iter->second;
}

CBase* CPrototypeManager::Find_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType, const wstring& strProtoKey)
{
	MAX_ENUM_CHECK_RETURN_NULLPTR(PROTOTYPE_TYPE::END, eProtoType);

	const auto& iter = m_mapPrototypeGlobal[Cast_Uint(eProtoType)].find(strProtoKey);
	if (iter == m_mapPrototypeGlobal[Cast_Uint(eProtoType)].end()) return nullptr;

	return iter->second;
}


CPrototypeManager* CPrototypeManager::Create(CONST PrototypeManager_Desc& tDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CPrototypeManager* pPrototypeManager = new CPrototypeManager(pDevice, pContext);


	if (FAILED(pPrototypeManager->Initialize(tDesc)))
	{
		Safe_Release(pPrototypeManager);
		MSG_BOX("Prototype Manager is failed to Create");
		return nullptr;
	}


	return pPrototypeManager;
}

void CPrototypeManager::Free()
{
	__super::Free();

	/* Prototype Manager Free() */
	for (int i = 0; i < (uint32)PROTOTYPE_TYPE::END; ++i)
	{
		for (auto& BaseVec : m_mapPrototype[i])
			for (auto& Pair : BaseVec)
				Safe_Release(Pair.second);
	}

	for (auto i = 0; i < Cast_Int(PROTOTYPE_TYPE::END); ++i)
		vector<map<const wstring, CBase*>>().swap(m_mapPrototype[i]);

	for (int i = 0; i < (uint32)PROTOTYPE_TYPE::END; ++i)
		for (auto& Proto_Global : m_mapPrototypeGlobal[i])
			Safe_Release(Proto_Global.second);
}
