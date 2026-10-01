#include "CCameraManager.h"
#include "CCameraObjectBase.h"
#include "CGameObject.h"
#include "CGameManager.h"
#include "CUICameraObject.h"
#include "CCameraComponent.h"

CCameraManager::CCameraManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext)
	: m_pMainCamera(nullptr), m_umapCamera(), m_vecUpdateCamera(), m_queueSwitchMainCamera()
	, m_pGameManager(CGameManager::GetInstance()), m_pDevice(pDevice), m_pDeviceContext(pDeviceContext)
	,m_pCurRenderingCamera(nullptr)
{
	Safe_AddRef(m_pGameManager);
}

HRESULT	CCameraManager::Initialize()
{
	return S_OK;
}




void CCameraManager::Pre_Update(float DT)
{


	return;
}


void CCameraManager::Update(float DT)
{
	/* CameraDepth 를 이용해서 정렬후 Update를 진행한다 */	
	Add_And_Sort_CameraUpdate(DT);

	if (m_vecUpdateCamera.empty())
		return;


	/* Update Vector 에 추가된 카메라 업데이트*/
	for (auto UpdateCamera : m_vecUpdateCamera)
	{
		UpdateCamera->Update(DT);
	}	
}

void CCameraManager::Late_Update(float DT)
{
	for (auto Camera : m_vecUpdateCamera)
	{
		Camera->Late_Update(DT);
		Safe_Release(Camera);
	}

	m_vecUpdateCamera.clear();

	/* Queue에 쌓아뒀던 메인 카메라 스위칭 하기 */
	ExecutePending_SwitchMainCamera();
}

HRESULT CCameraManager::Render()
{
	/* Render 할 카메라 와 순서 세팅 */
	Add_And_Sort_CameraRender();

	HRESULT	hr{ E_FAIL };

	if (m_vecRenderCamera.empty()) return S_OK;


	/* ---- measurement : F6 cycles SHARED -> APLAN -> PERCAM ---- */
	if (m_pGameManager->InputManager_GetButtonUp(KEY_TYPE::F6))
		m_pGameManager->RenderManager_Set_RenderMode((m_pGameManager->RenderManager_Get_RenderMode() + 1) % 3);

	m_pGameManager->RenderManager_GpuTimer_Begin(Cast_Uint(m_vecRenderCamera.size()));

	CONST int32 iMode = m_pGameManager->RenderManager_Get_RenderMode();

	if (iMode == 2)			/* PERCAM */
	{
		hr = S_OK;
		for (auto& RenderCam : m_vecRenderCamera)
		{
			if (!RenderCam) continue;
			m_pCurRenderingCamera = RenderCam;
			if (FAILED(hr = m_pGameManager->RenderManager_RenderGruop(RenderCam)))
				break;
		}
#ifdef _DEBUG
		if (SUCCEEDED(hr))
			hr = m_pGameManager->RenderManager_Render_DebugPass(m_vecRenderCamera);
#endif
	}
	else if (iMode == 1)	/* APLAN */
	{
		hr = m_pGameManager->RenderManager_RenderGruop_APlan(m_vecRenderCamera);
	}
	else					/* SHARED */
	{
		hr = m_pGameManager->RenderManager_RenderGruop(m_vecRenderCamera);
	}

	m_pGameManager->RenderManager_GpuTimer_End();

	m_vecRenderCamera.clear();
	m_pCurRenderingCamera = nullptr;


	return hr;
}


HRESULT CCameraManager::Set_MainCameraTarget(CGameObject* pGameObject)
{
	NULLPTR_RETURN_EFAIL(pGameObject);

	m_pMainCamera->Set_TargetObject(pGameObject);

	return S_OK;
}

HRESULT CCameraManager::Set_SubCameraTarget(const wstring& wstrKey , UPDATE_MODE eUpdateMode, CCameraObjectBase* pGameObject)
{
	NULLPTR_RETURN_EFAIL(pGameObject);

	auto pFindCamera = Find_Camera(wstrKey, eUpdateMode);

	NULLPTR_RETURN_EFAIL(pFindCamera);

	pFindCamera->Set_TargetObject(pGameObject);

	return S_OK;
}

void CCameraManager::Set_CameraRenderActive(const wstring& wstrKey, UPDATE_MODE eUpdateMode, bool isRednerActive)
{
	auto pFindCamera = Find_Camera(wstrKey, eUpdateMode);
	NULLPTR_RETURN_NULL(pFindCamera);

	pFindCamera->Set_UpdatePhaseActive(UPDATE_PHASE::Render, isRednerActive);

}


HRESULT CCameraManager::Add_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode, CCameraObjectBase* pCamera)
{
	NULLPTR_RETURN_EFAIL(pCamera);

	CCameraObjectBase* pFindCamera = Find_Camera(wstrKey,eUpdateMode);

	if (pFindCamera != nullptr)
		return E_FAIL;


	m_umapCamera[Cast_Uint(eUpdateMode)].emplace(wstrKey, pCamera);

	if (m_pMainCamera == nullptr)
	{
		m_pMainCamera = pCamera;
		Safe_AddRef(pCamera);
	}

	return S_OK;
}

HRESULT CCameraManager::Remove_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode)
{
	auto& CameraMap = m_umapCamera[Cast_Uint(eUpdateMode)];
	auto iter = CameraMap.find(wstrKey);
	if (iter == CameraMap.end())
		return E_FAIL;

	CCameraObjectBase* pRemoveCamera = iter->second;
	if (m_pMainCamera == pRemoveCamera)
	{
		Safe_Release(m_pMainCamera);
		m_pMainCamera = nullptr;
	}

	CameraMap.erase(iter);
	Safe_Release(pRemoveCamera);

	if (m_pMainCamera == nullptr)
	{
		for (auto& CameraMapList : m_umapCamera)
		{
			if (CameraMapList.empty())
				continue;

			m_pMainCamera = CameraMapList.begin()->second;
			Safe_AddRef(m_pMainCamera);
			break;
		}
	}

	return S_OK;
}



void CCameraManager::Requset_SwitchMainCamera(const wstring& strKey, UPDATE_MODE eUpdateMode)
{
	if (eUpdateMode == UPDATE_MODE::STATIC)
		return;

	m_queueSwitchMainCamera.push([this, strKey, eUpdateMode]() -> HRESULT { return Switch_MainCamera(strKey, eUpdateMode); });
}

HRESULT CCameraManager::Switch_MainCamera(const wstring& strKey, UPDATE_MODE eUpdateMode)
{
	CCameraObjectBase* pCamera = Find_Camera(strKey, eUpdateMode);

	NULLPTR_RETURN_EFAIL(pCamera);

	if (m_pMainCamera == pCamera)
		return E_FAIL;

	/* 원래 가지고 있던 메인 카메라의 레퍼런스 감소 후 새로운 메인카메라 레퍼런스 증가*/
	Safe_Release(m_pMainCamera);
	m_pMainCamera = pCamera;
	Safe_AddRef(m_pMainCamera);

	return S_OK;
}

void CCameraManager::ExecutePending_SwitchMainCamera()
{
	if (m_queueSwitchMainCamera.empty())
		return;

	m_queueSwitchMainCamera.front().operator()();
	m_queueSwitchMainCamera.pop();
}

void CCameraManager::Add_And_Sort_CameraUpdate(float DT)
{
	for (auto& DyCam : m_umapCamera[Cast_Uint(UPDATE_MODE::DYNAMIC)])
		if (DyCam.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::Update) != false)
		{
			m_vecUpdateCamera.push_back(DyCam.second);
			Safe_AddRef(DyCam.second);
		}


	if (m_vecUpdateCamera.size() <= 1) return;


	std::sort(m_vecUpdateCamera.begin(), m_vecUpdateCamera.end(), [](const CCameraObjectBase* A, const CCameraObjectBase* B) {
		return A->Get_CameraUpdateDepth() < B->Get_CameraUpdateDepth();
		});
}

void CCameraManager::Add_And_Sort_CameraRender()
{
	/* 미리 추가를 하고 카메라 Depth에 따라서 추가로 Render 순서 Sorting */

	for (uint32 i = 0; i < Cast_Uint(UPDATE_MODE::END); ++i)
	{
		for (auto Camera : m_umapCamera[i])
			if (Camera.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render) == true)
			{
				m_vecRenderCamera.push_back(Camera.second);
			}
	}

	std::sort(m_vecRenderCamera.begin(), m_vecRenderCamera.end(), [](const CCameraObjectBase* A, const CCameraObjectBase* B)
		{
			/* 내림차순 */
			return A->Get_CameraRenderDepth() < B->Get_CameraRenderDepth();
		});
}


CCameraObjectBase* CCameraManager::Get_Camera(const wstring& strKey, UPDATE_MODE eUpdateMode)
{
	return Find_Camera(strKey,eUpdateMode);
}

HRESULT CCameraManager::MakeDefaultCamera(const CDefaultCameraObject::DefaultCameraObject_Dest* pDefaultCameraDest)
{
	CDefaultCameraObject* pDefaultCamera = CDefaultCameraObject::Create(m_pDevice, m_pDeviceContext , (void*)pDefaultCameraDest);
	
	NULLPTR_RETURN_EFAIL(pDefaultCamera);

	m_umapCamera[Cast_Uint(pDefaultCameraDest->eUpdateMode)].emplace(pDefaultCamera->GetName(), pDefaultCamera);

	return S_OK;
}

HRESULT CCameraManager::MakeDefaultUICamera(const CUICameraObject::UICameraObject_Desc* pUICameraDest)
{
	CUICameraObject* pUICamera = CUICameraObject::Create(m_pDevice, m_pDeviceContext, (void*)pUICameraDest);

	NULLPTR_RETURN_EFAIL(pUICamera);

	m_umapCamera[Cast_Uint(pUICameraDest->eUpdateMode)].emplace(pUICamera->GetName(), pUICamera);

	return S_OK;
}

CCameraObjectBase* CCameraManager::Find_Camera(const wstring& strKey, UPDATE_MODE eUpdateMode)
{
	auto iter = m_umapCamera[Cast_Uint(eUpdateMode)].find(strKey);

	if (iter == m_umapCamera[Cast_Uint(eUpdateMode)].end())
		return nullptr;


	return iter->second;

}

CCameraManager* CCameraManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext)
{
	CCameraManager* mCameraManager = new CCameraManager(pDevice,pDeviceContext);

	INIT_FAILED_CEHCK(mCameraManager);

	return mCameraManager;
}

void CCameraManager::Free()
{
	__super::Free();


	/*  CCameraManager::Free() */
	Safe_Release(m_pMainCamera);
	for ( auto& CameraMapList : m_umapCamera)
		for (auto& Camera : CameraMapList)
			Safe_Release(Camera.second);


	Safe_Release(m_pGameManager);
}
