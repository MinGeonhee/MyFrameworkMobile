#include "CGameObjectManager.h"
#include "CGameManager.h"
#include "CGameObject.h"
#include "CLayer.h"
#include "CRenderManager.h"


CGameObjectManager::CGameObjectManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_iTotalSceneNum(0) , m_pGameManager(CGameManager::GetInstance())
{
	Safe_AddRef(m_pGameManager);
}

HRESULT CGameObjectManager::Initialize(CONST GameObjectManager_Desc& tDesc)
{
	vector<map<const wstring, CLayer*>>().swap(m_mapLayer);

	m_iTotalSceneNum = tDesc.iTotalSceneNum;
	
	/* Layer 개수가 미리 할당된다 */
	m_mapLayer.resize(m_iTotalSceneNum);

	return S_OK;
}

CLayer* CGameObjectManager::Find_Layer(uint32 iFindSceneID, const wstring& strFindLayerKey)
{
	MAX_ENUM_CHECK_RETURN_NULLPTR(m_iTotalSceneNum, iFindSceneID);

	auto iter = m_mapLayer[iFindSceneID].find(strFindLayerKey);

	if (iter == m_mapLayer[iFindSceneID].end())
		return nullptr;

	return iter->second;
}

CLayer* CGameObjectManager::Find_Layer_InGlobal(const wstring& strFindLayerKey)
{
	const auto& iter = m_mapGlobalLayers.find(strFindLayerKey);
	if (iter == m_mapGlobalLayers.end()) return nullptr;

	return iter->second;
}

CGameObject* CGameObjectManager::Find_GameObject_InGlobal(const wstring& strFindLayerKey, const wstring& strGameObjectName)
{
	CLayer* pFindLayer = Find_Layer_InGlobal(strFindLayerKey);
	NULLPTR_RETURN_NULLPTR(pFindLayer);

	return pFindLayer->Find_GameObject(strGameObjectName);
}

CGameObject* CGameObjectManager::Find_GameObject_InGlobal(const wstring& strFindLayerKey, CGameObject* pFindGameObject)
{
	NULLPTR_RETURN_NULLPTR(pFindGameObject);

	CLayer* pLayer{ nullptr };
	pLayer = Find_Layer_InGlobal(strFindLayerKey);
	
	return pLayer->Find_GameObject(pFindGameObject);
}

CGameObject* CGameObjectManager::Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, CGameObject* pFindGameObject)
{
	NULLPTR_RETURN_NULLPTR(pFindGameObject);

	CLayer* pFindLayer = Find_Layer(iFindSceneID, strFindLayerKey);
	NULLPTR_RETURN_NULLPTR(pFindLayer);

	return pFindLayer->Find_GameObject(pFindGameObject);
}

CGameObject* CGameObjectManager::Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, const wstring& strGameObjectName)
{
	CLayer* pFindLayer = Find_Layer(iFindSceneID,strFindLayerKey);
	NULLPTR_RETURN_NULLPTR(pFindLayer);

	return pFindLayer->Find_GameObject(strGameObjectName);
}

CGameObject* CGameObjectManager::Add_GameObject(uint32 iFindProtoSceneID, const wstring& strCloneProtokey, uint32 iAddCloneSceneID, const wstring& strAddLayerKey, void* pArg)
{
	CGameObject* pGameObject = static_cast<CGameObject*>(m_pGameManager->PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE::GAMEOBJECT,iFindProtoSceneID, strCloneProtokey, pArg));

	CLayer* pLayer = Find_Layer(iFindProtoSceneID, strAddLayerKey);

	if (pLayer == nullptr)
	{
		pLayer = CLayer::Create();
		NULLPTR_RETURN_NULLPTR(pLayer);
		m_mapLayer[iAddCloneSceneID].emplace(strAddLayerKey,pLayer);
	}

	pLayer->Add_GameObject(pGameObject);

	return pGameObject;
}

CGameObject* CGameObjectManager::Add_GameObject_ByGlobalProto(const wstring& strFindProtoKey, const wstring& strAddLayerKey, void* pArg, uint32 iAddCloneSceneID)
{
	CGameObject* pGameObject = static_cast<CGameObject*>(m_pGameManager->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::GAMEOBJECT, strFindProtoKey, pArg));

	CLayer* pLayer{ nullptr };

	if (iAddCloneSceneID == GLOBAL)		{ pLayer = Find_Layer_InGlobal(strAddLayerKey); }
	else												{ pLayer = Find_Layer(iAddCloneSceneID, strAddLayerKey); }

	if (pLayer == nullptr)
	{
		pLayer = CLayer::Create();
		NULLPTR_RETURN_NULLPTR(pLayer);

		if (iAddCloneSceneID == GLOBAL) { m_mapGlobalLayers.emplace(strAddLayerKey,pLayer); }
		else										 { m_mapLayer[iAddCloneSceneID].emplace(strAddLayerKey, pLayer); }
	}

	pLayer->Add_GameObject(pGameObject);

	return pGameObject;
}

void CGameObjectManager::Pre_Update(uint32 iSceneID, float DT)
{
	MAX_ENUM_CHECK_RETURN_NULL(m_iTotalSceneNum, iSceneID);
	
	for (const auto& Pair : m_mapGlobalLayers)
		Pair.second->Pre_Update(DT);

	for (const auto& Pair : m_mapLayer[iSceneID])
		Pair.second->Pre_Update(DT);
}

void CGameObjectManager::Update(uint32 iSceneID, float DT)
{
	MAX_ENUM_CHECK_RETURN_NULL(m_iTotalSceneNum, iSceneID);

	for (const auto& Pair : m_mapGlobalLayers)
		Pair.second->Update(DT);


	for (const auto& Pair : m_mapLayer[iSceneID])
		Pair.second->Update(DT);
}

void CGameObjectManager::Late_Update(uint32 iSceneID, float DT)
{
	MAX_ENUM_CHECK_RETURN_NULL(m_iTotalSceneNum, iSceneID);

	for (const auto& Pair : m_mapGlobalLayers)
		Pair.second->Late_Update(DT);

	for (const auto& Pair : m_mapLayer[iSceneID])
		Pair.second->Late_Update(DT);
}

HRESULT CGameObjectManager::Add_RenderGroup(uint32 iSceneID)
{
	for (auto& Pair : m_mapGlobalLayers)
		if (FAILED(Pair.second->Add_RenderGroup()))
			return E_FAIL;

	for (auto& Pair : m_mapLayer[iSceneID])
		if (FAILED(Pair.second->Add_RenderGroup()))
			return E_FAIL;

	return S_OK;
}

void CGameObjectManager::Clean_DeadObject()
{
	for (auto& Pair : m_mapGlobalLayers)
		Pair.second->Clean_DeadObject();

	for (auto& MapLayers : m_mapLayer)
		for (auto& MapLayer : MapLayers)
			MapLayer.second->Clean_DeadObject();		
}

void CGameObjectManager::Add_ColliderGroup(uint32 iSceneID)
{
	MAX_ENUM_CHECK_RETURN_NULL(m_iTotalSceneNum, iSceneID);

	for (const auto& Pair : m_mapGlobalLayers)
		if (Pair.second)
			Pair.second->Add_ColliderGroup();

	for (const auto& Pair : m_mapLayer[iSceneID])
		if (Pair.second)
			Pair.second->Add_ColliderGroup();

}


const CLayer* CGameObjectManager::Get_Layer(uint32 iSceneID, const wstring& wstrLayerName)
{
	if (iSceneID >= m_iTotalSceneNum) return nullptr;


	auto iter = m_mapLayer[iSceneID].find(wstrLayerName);

	if (iter == m_mapLayer[iSceneID].end()) return nullptr;

	return iter->second;
}

HRESULT CGameObjectManager::Clear_GameObjectByScneeID(uint32 iSceneID)
{
	MAX_ENUM_CHECK_RETURN_EFAIL(m_iTotalSceneNum, iSceneID);

	for (auto& Pair : m_mapLayer[iSceneID])
		Safe_Release(Pair.second);

	m_mapLayer[iSceneID].clear();

	map<const wstring, CLayer*>().swap(m_mapLayer[iSceneID]);

	return S_OK;
}


CGameObjectManager* CGameObjectManager::Create( CONST GameObjectManager_Desc& tDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	
	CGameObjectManager* pGameObjectManager = new CGameObjectManager(pDevice, pContext);


	if (FAILED(pGameObjectManager->Initialize(tDesc)))
	{
		Safe_Release(pGameObjectManager);
		MSG_BOX("Prototype Manager is failed to Create");
		return nullptr;
	}

	return pGameObjectManager;
}

void	CGameObjectManager::Free()
{
	__super::Free();

	/* My Free() */
	for (auto& MapLayer : m_mapLayer)
	{
		for (auto& Pair : MapLayer)
			Safe_Release(Pair.second);
		MapLayer.clear();
		map<const wstring, CLayer*>().swap(MapLayer);
	}

	for (auto& MapGlobalLayer : m_mapGlobalLayers)
	{
		Safe_Release(MapGlobalLayer.second);
	}

	m_mapGlobalLayers.clear();



	Safe_Release(m_pGameManager);
}



