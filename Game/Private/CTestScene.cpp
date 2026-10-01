#include "pch.h"
#include "CTestScene.h"

#include "CGameManager.h"
#include "CDefaultCameraObject.h"
#include "CLight.h"
#include "CTransform.h"
#include "CTestObject.h"
#include "Game_Entry.h"

namespace
{
	const wchar_t* CAMERA_NAME = L"Camera_Free";
}

CTestScene::CTestScene(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CScene(pDevice, pContext)
{
	/* 이전 Scene 삭제 후 쌓기 */
	m_tSceneInfo.ePopFlag = SCENE_POPFLAG::Scene_Pop_Default;
	m_tSceneInfo.iSceneID = Cast_Uint(Game::SCENEID::TEST);
	m_tSceneInfo.eStackType = SCENE_STACK_TYPE::REPLACE;
	m_tSceneInfo.eRenderFlag = SCENE_RENDERFLAG::Scene_Render_OnlyThisScene;
}

HRESULT CTestScene::Initialize(const CreateScene_Desc& tCreateSceneDesc)
{
	if (FAILED(__super::Initialize(tCreateSceneDesc)))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_Object()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTestScene::Ready_Camera()
{
	/* 이미 카메라가 있다면 (씬 재진입) 그대로 사용 */
	if (m_pGameManager->CameraManager_Get_Camera(CAMERA_NAME, UPDATE_MODE::DYNAMIC) != nullptr)
		return S_OK;

	const VIEWPORT_DESC& tViewPort = m_pGameManager->GraphicDevice_Get_OriginViewPort();

	CDefaultCameraObject::DefaultCameraObject_Dest tCameraDesc{};
	tCameraDesc.GameObect_Desc_strGameObjectName = CAMERA_NAME;
	tCameraDesc.eUpdateMode = UPDATE_MODE::DYNAMIC;
	tCameraDesc.eCameraType = CAMERA_TYPE::FREE;
	tCameraDesc.eMoveType = MOVE_TYPE::NORMAL;
	tCameraDesc.pActor = nullptr;
	tCameraDesc.tTransform = { 0.f, 4.f, -8.f };
	tCameraDesc.tRotation = { XMConvertToRadians(20.f), 0.f, 0.f };
	tCameraDesc.fSpeedPerSec = 8.f;
	tCameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	tCameraDesc.CameraObjectBase_pRTVCom = m_pGameManager->GraphicDevice_Get_RTV();
	tCameraDesc.CameraObjectBase_pDepthStencilViewCom = m_pGameManager->GraphicDevice_Get_DSV();
	tCameraDesc.CameraObjectBase_isUpdateActive = true;
	tCameraDesc.CameraObjectBase_isRenderActive = true;
	tCameraDesc.CameraObjectBase_eCameraUpdateDepth = CAMERA_UPDATE_DEPTH::DEPTH_1;
	tCameraDesc.CameraObjectBase_eCameraRenderDepth = CAMERA_RENDER_DEPTH::DEPTH_1;
	tCameraDesc.CameraObjectBase_eProjectonType = PROJECTION_TYPE::PERSPECTIVE;
	tCameraDesc.CameraObjectBase_fScreenStartX = tViewPort.TopLeftX;
	tCameraDesc.CameraObjectBase_fScreenStartY = tViewPort.TopLeftY;
	tCameraDesc.CameraObjectBase_fFilmingSizeX = tViewPort.Width;
	tCameraDesc.CameraObjectBase_fFilmingSizeY = tViewPort.Height;
	tCameraDesc.CameraObjectBase_fFov = 60.f;
	tCameraDesc.CameraObjectBase_fNear = 0.1f;
	tCameraDesc.CameraObjectBase_fFar = 1000.f;

	return m_pGameManager->CameraManager_MakeDefaultCamera(&tCameraDesc);
}

HRESULT CTestScene::Ready_Object()
{
	const uint32 iSceneID = m_tSceneInfo.iSceneID;

	/* 바닥 타일 */
	for (int32 z = -3; z <= 3; ++z)
	{
		for (int32 x = -3; x <= 3; ++x)
		{
			CTestObject::TESTOBJECT_DESC tDesc{};
			tDesc.GameObect_Desc_strGameObjectName = L"Tile";
			tDesc.wstrModelKey = L"Model_Tile";
			tDesc.wstrShaderKey = L"Shader_VtxModel";
			tDesc.strPass = "Default";
			tDesc.tTransform = { x * 2.f, 0.f, z * 2.f };
			if (m_pGameManager->GameObjectManager_Add_GameObject_ByGlobalProto(L"Proto_GameObject_TestObject", L"Layer_Field", &tDesc, iSceneID) == nullptr)
				return E_FAIL;
		}
	}

	/* 바위 / 보물상자 */
	const Float3 arrRockPos[] = { { -4.f, 0.f, 3.f }, { 4.5f, 0.f, 2.f }, { -3.f, 0.f, -4.f } };
	for (const Float3& vPos : arrRockPos)
	{
		CTestObject::TESTOBJECT_DESC tDesc{};
		tDesc.GameObect_Desc_strGameObjectName = L"Rock";
		tDesc.wstrModelKey = L"Model_Rock";
		tDesc.wstrShaderKey = L"Shader_VtxModel";
		tDesc.tTransform = vPos;
		if (m_pGameManager->GameObjectManager_Add_GameObject_ByGlobalProto(L"Proto_GameObject_TestObject", L"Layer_Object", &tDesc, iSceneID) == nullptr)
			return E_FAIL;
	}

	{
		CTestObject::TESTOBJECT_DESC tDesc{};
		tDesc.GameObect_Desc_strGameObjectName = L"TreasureBox";
		tDesc.wstrModelKey = L"Model_TreasureBox";
		tDesc.wstrShaderKey = L"Shader_VtxModel";
		tDesc.tTransform = { 3.f, 0.f, -2.f };
		tDesc.fTurnSpeed = 0.5f;
		if (m_pGameManager->GameObjectManager_Add_GameObject_ByGlobalProto(L"Proto_GameObject_TestObject", L"Layer_Object", &tDesc, iSceneID) == nullptr)
			return E_FAIL;
	}

	/* 애니메이션 모델 */
	{
		CTestObject::TESTOBJECT_DESC tDesc{};
		tDesc.GameObect_Desc_strGameObjectName = L"Link";
		tDesc.wstrModelKey = L"Model_Link";
		tDesc.wstrShaderKey = L"Shader_VtxAnimModel";
		tDesc.isAnim = true;
		tDesc.iAnimIndex = 0;
		tDesc.fAnimSpeed = 1.f;
		tDesc.tTransform = { 0.f, 0.f, 0.f };
		tDesc.tRotation = { 0.f, XM_PI, 0.f };
		m_pLink = static_cast<CTestObject*>(m_pGameManager->GameObjectManager_Add_GameObject_ByGlobalProto(L"Proto_GameObject_TestObject", L"Layer_Actor", &tDesc, iSceneID));
		NULLPTR_RETURN_EFAIL(m_pLink);
	}

	return S_OK;
}

void CTestScene::Update(float DT)
{
	CScene::Update(DT);

	/* 애니메이션 순환 확인 (Space / 5초마다) */
	m_fAccTime += DT;
	if (m_pLink != nullptr && m_pLink->Get_AnimationCount() > 0 &&
		(m_pGameManager->InputManager_GetButtonDown(KEY_TYPE::SPACE) || m_fAccTime > 5.f))
	{
		m_fAccTime = 0.f;
		m_iAnimIndex = (m_iAnimIndex + 1) % m_pLink->Get_AnimationCount();
		m_pLink->Change_Animation(m_iAnimIndex);
	}

	/* 점광원 하나를 원 궤도로 움직인다 */
	m_fLightTime += DT;
	if (CLight* pLight = m_pGameManager->LightManager_Get_Light(LIGHT_TYPE::POINT, 0))
		pLight->Set_Position(Float3(cosf(m_fLightTime) * 4.f, 2.f, sinf(m_fLightTime) * 4.f));
}

HRESULT CTestScene::Render()
{
	wstring wstrText = L"MyFramework Mobile  FPS : " + std::to_wstring(Game::Get_FPS());
	m_pGameManager->FontMananger_Draw(L"156", wstrText.c_str(), Float2(16.f, 16.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), FONT_RENDER_TYPE::DEFUALT);

	wstring wstrAnim = L"Animation : " + std::to_wstring(m_iAnimIndex) + L" / " + std::to_wstring(m_pLink != nullptr ? m_pLink->Get_AnimationCount() : 0);
	m_pGameManager->FontMananger_Draw(L"156", wstrAnim.c_str(), Float2(16.f, 56.f), XMVectorSet(1.f, 0.85f, 0.2f, 1.f), FONT_RENDER_TYPE::DEFUALT);

	return S_OK;
}

void CTestScene::Ready_CollisionTable()
{
}

void CTestScene::Ready_Light()
{
	CLight::Light_Desc tLightDesc{};
	tLightDesc.eLightType = LIGHT_TYPE::DIRECTIONAL;
	tLightDesc.tLightBufferData.vDirection = Float4(1.f, -1.f, 1.f, 0.f);
	tLightDesc.tLightBufferData.vDiffuse = Float4(0.8f, 0.8f, 0.8f, 1.f);
	tLightDesc.tLightBufferData.vAmbient = Float4(0.4f, 0.4f, 0.4f, 1.f);
	tLightDesc.tLightBufferData.vSpecular = Float4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pGameManager->LightManager_Add_Light(tLightDesc);

	CLight::Light_Desc tPointDesc{};
	tPointDesc.eLightType = LIGHT_TYPE::POINT;
	tPointDesc.tLightBufferData.vPosition = Float4(3.f, 2.f, 0.f, 1.f);
	tPointDesc.tLightBufferData.vDiffuse = Float4(1.f, 0.4f, 0.1f, 1.f);
	tPointDesc.tLightBufferData.vAmbient = Float4(0.f, 0.f, 0.f, 1.f);
	tPointDesc.tLightBufferData.vSpecular = Float4(0.5f, 0.3f, 0.1f, 1.f);
	tPointDesc.tLightBufferData.fRange = 6.f;
	m_pGameManager->LightManager_Add_Light(tPointDesc);

	tPointDesc.tLightBufferData.vPosition = Float4(-4.f, 2.f, -2.f, 1.f);
	tPointDesc.tLightBufferData.vDiffuse = Float4(0.2f, 0.5f, 1.f, 1.f);
	tPointDesc.tLightBufferData.vSpecular = Float4(0.1f, 0.3f, 0.5f, 1.f);
	m_pGameManager->LightManager_Add_Light(tPointDesc);
}

CTestScene* CTestScene::Create(const CreateScene_Desc& tCreateSceneDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CTestScene* pScene = new CTestScene(pDevice, pContext);

	if (FAILED(pScene->Initialize(tCreateSceneDesc)))
	{
		Safe_Release(pScene);
		MSG_BOX("TestScene is failed to Create");
		return nullptr;
	}

	return pScene;
}

void CTestScene::Free()
{
	__super::Free();
}
