#include "pch.h"
#include "Game_Entry.h"

#include "CGameManager.h"
#include "CShader.h"
#include "CTexture.h"
#include "CModel.h"
#include "CBlendState.h"
#include "CDepthstencilState.h"

#include "CTestScene.h"
#include "CTestObject.h"

namespace
{
	CGameManager*			g_pGameManager = nullptr;
	ComPtr<Device>			g_pDevice;
	ComPtr<DeviceContext>	g_pContext;
	void					(*g_pfnBeforeEndFrame)() = nullptr;

	/* 원본 Client 경로 규칙 : 실행 폴더(Client/Bin) 기준 "../../" = 저장소 루트 */
	const wchar_t* SHADER_PATH = L"../../Game/Shader/";

	HRESULT Ready_Shader()
	{
		struct SHADER_INFO
		{
			const wchar_t*				pKey;
			const wchar_t*				pFile;
			const INPUT_ELEMENT_DESC*	pElements;
			uint32						iNumElements;
		};

		const SHADER_INFO arrShader[] = {
			{ L"Shader_VtxPosTex",		L"Shader_VtxPosTex.hlsl",		VTXPOSTEX().GetInputElementDesc().data(),	VTXPOSTEX().GetInputElementDescSize() },
			{ L"Shader_VtxNorTex",		L"Shader_VtxNorTex.hlsl",		VTXNORTEX().GetInputElementDesc().data(),	VTXNORTEX().GetInputElementDescSize() },
			{ L"Shader_VtxModel",		L"Shader_VtxModel.hlsl",		VTXMESH().GetInputElementDesc().data(),		VTXMESH().GetInputElementDescSize() },
			{ L"Shader_VtxAnimModel",	L"Shader_VtxAnimModel.hlsl",	VTXANIMMESH().GetInputElementDesc().data(),	VTXANIMMESH().GetInputElementDescSize() },
			{ L"Shader_ClientUI",		L"Shader_ClientUI.hlsl",		VTXPOSTEX().GetInputElementDesc().data(),	VTXPOSTEX().GetInputElementDescSize() },
		};

		for (const SHADER_INFO& tInfo : arrShader)
		{
			CShader* pShader = CShader::Create(wstring(SHADER_PATH) + tInfo.pFile, tInfo.pElements, tInfo.iNumElements, g_pDevice, g_pContext);
			NULLPTR_RETURN_EFAIL(pShader);
			if (FAILED(g_pGameManager->ResourceManager_Add_Resource(tInfo.pKey, pShader)))
				return E_FAIL;
		}

		CShader* pAnimShader = static_cast<CShader*>(g_pGameManager->ResourceManager_Get_Resource(L"Shader_VtxAnimModel", RESOURCE_TYPE::SHADER, false));
		NULLPTR_RETURN_EFAIL(pAnimShader);
		pAnimShader->Add_EffectVariable(EFFECT_VAL_TYPE::MATRIX, Shader_ArrayBoneMatrix);
		Safe_Release(pAnimShader);

		return S_OK;
	}

	HRESULT Ready_Model()
	{
		struct MODEL_INFO
		{
			const wchar_t* pKey;
			const wchar_t* pPath;
			float          fScale;
		};

		const MODEL_INFO arrModel[] = {
			{ L"Model_Link",		L"../../Resource/Model/Anim/Link/Link_ModelInfo.json",					1.f },
			{ L"Model_Tile",		L"../../Resource/Model/NonAnim/Tile/Tile_ModelInfo.json",				1.f },
			{ L"Model_Rock",		L"../../Resource/Model/NonAnim/Rock/Rock_ModelInfo.json",				1.f },
			{ L"Model_TreasureBox",	L"../../Resource/Model/NonAnim/TreasureBox/TreasureBox_ModelInfo.json",	1.f },
		};

		for (const MODEL_INFO& tInfo : arrModel)
		{
			CModel* pModel = CModel::Create(tInfo.pPath, XMMatrixScaling(tInfo.fScale, tInfo.fScale, tInfo.fScale), g_pDevice, g_pContext);
			NULLPTR_RETURN_EFAIL(pModel);
			if (FAILED(g_pGameManager->ResourceManager_Add_Resource(tInfo.pKey, pModel)))
				return E_FAIL;
		}
		return S_OK;
	}

	HRESULT Ready_Font()
	{
		if (FAILED(g_pGameManager->FontMananger_Add_Font(16, L"156", L"../../Resource/Font/156.spritefont")))
			return E_FAIL;

		return S_OK;
	}

	HRESULT Ready_RenderState()
	{
		/* 원본 Client Ready_RenderState 와 같은 상태들 (D3D11 Desc → Diligent Desc) */
		DEPTH_STENCIL_DESC tDssState{};
		tDssState.DepthEnable = true;
		tDssState.DepthWriteEnable = true;
		tDssState.DepthFunc = Diligent::COMPARISON_FUNC_LESS;
		g_pGameManager->RenderStateManager_Add_RenderState(Cast_Uint(Game::DSS_TYPE::Depth_Write_On), CDepthStencilState::Create(g_pDevice, g_pContext, tDssState));

		tDssState.DepthWriteEnable = false;		// 기록만 끔
		g_pGameManager->RenderStateManager_Add_RenderState(Cast_Uint(Game::DSS_TYPE::Depth_Write_Off), CDepthStencilState::Create(g_pDevice, g_pContext, tDssState));

		BLEND_DESC tBlendDesc{};
		g_pGameManager->RenderStateManager_Add_RenderState(Cast_Uint(Game::BLEND_TYPE::NON_BLEND), CBlendState::Create(g_pDevice, g_pContext, tBlendDesc));

		/* Alpha Blend */
		BLEND_DESC AlphaDesc{};
		AlphaDesc.RenderTargets[0].BlendEnable = true;
		AlphaDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
		AlphaDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		AlphaDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
		AlphaDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
		AlphaDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
		AlphaDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
		for (uint32 i = 1; i < 8; ++i)
			AlphaDesc.RenderTargets[i] = AlphaDesc.RenderTargets[0];
		g_pGameManager->RenderStateManager_Add_RenderState(Cast_Uint(Game::BLEND_TYPE::BLEND_ALPHA), CBlendState::Create(g_pDevice, g_pContext, AlphaDesc));

		/* Add Blend */
		BLEND_DESC AddDesc = AlphaDesc;
		AddDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_ONE;
		AddDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_ONE;
		g_pGameManager->RenderStateManager_Add_RenderState(Cast_Uint(Game::BLEND_TYPE::BLEND_ADD), CBlendState::Create(g_pDevice, g_pContext, AddDesc));

		return S_OK;
	}

	HRESULT Ready_Prototype()
	{
		if (FAILED(g_pGameManager->PrototypeManager_Add_Prototype_InGlobal(PROTOTYPE_TYPE::GAMEOBJECT, L"Proto_GameObject_TestObject", CTestObject::Create(g_pDevice, g_pContext))))
			return E_FAIL;

		return S_OK;
	}

	HRESULT Ready_RegisterSceneFactory()
	{
		ComPtr<Device> pDevice = g_pDevice;
		ComPtr<DeviceContext> pContext = g_pContext;

		if (FAILED(g_pGameManager->SceneManager_Register_SceneFactory(Cast_Uint(Game::SCENEID::TEST),
			[=](const CreateScene_Desc& tCreateScene_Desc) -> CScene* { return CTestScene::Create(tCreateScene_Desc, pDevice, pContext); })))
			return E_FAIL;

		return S_OK;
	}

	HRESULT Ready_Scene(Game::SCENEID eType)
	{
		CreateScene_Desc tDesc{};
		tDesc.eIsPop = SCENE_ISPOP::NO;
		tDesc.iMakeSceneNum = Cast_Uint(eType);
		tDesc.isNeedSceneOverLay = false;

		g_pGameManager->SceneManager_RequestChangeScene(tDesc);
		return S_OK;
	}
}

namespace Game
{
	HRESULT Initialize(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI,
					   ComPtr<Device>* ppDevice, ComPtr<DeviceContext>* ppContext)
	{
		g_pGameManager = CGameManager::GetInstance();
		Safe_AddRef(g_pGameManager);

		Engine_Desc tEngineDesc{};
		/* GameObject/Render/Prototype Manager Desc */
		tEngineDesc.tGameObjectManager_Desc.iTotalSceneNum = Cast_Uint(SCENEID::END);
		tEngineDesc.tPrototypeManager_Desc.iTotalSceneNum = Cast_Uint(SCENEID::END);

		/* GraphicDevice Desc */
		tEngineDesc.tGraphicDevice_Desc.hWnd = hWnd;
		tEngineDesc.tGraphicDevice_Desc.hInst = hInst;
		tEngineDesc.tGraphicDevice_Desc.iWinSizeX = iWinSizeX;
		tEngineDesc.tGraphicDevice_Desc.iWinSizeY = iWinSizeY;
		tEngineDesc.tGraphicDevice_Desc.eWindowMode = WINMODE::WIN;
		tEngineDesc.tGraphicDevice_Desc.eGraphicAPI = eGraphicAPI;

		/* Collision Manager */
		tEngineDesc.tCollisionManager_Desc.iMaxColliderGroupID = Cast_Uint(COLLIDER_GROUP_ID::END);

		if (FAILED(g_pGameManager->Initialize_Engine(tEngineDesc, &g_pDevice, &g_pContext)))
			return E_FAIL;

		if (ppDevice != nullptr)	*ppDevice = g_pDevice;
		if (ppContext != nullptr)	*ppContext = g_pContext;

		srand(static_cast<uint32>(time(nullptr)));

		if (FAILED(g_pGameManager->TimerManager_Add_Timer(L"Timer_Default")))
			return E_FAIL;
		if (FAILED(g_pGameManager->TimerManager_Add_Timer(L"Timer_60")))
			return E_FAIL;

		if (FAILED(g_pGameManager->Reaydy_Default(g_pDevice, g_pContext)))
			return E_FAIL;

		if (FAILED(Ready_Font()))			return E_FAIL;
		if (FAILED(Ready_Shader()))			return E_FAIL;
		if (FAILED(Ready_RenderState()))	return E_FAIL;
		if (FAILED(Ready_Model()))			return E_FAIL;
		if (FAILED(Ready_Prototype()))		return E_FAIL;
		if (FAILED(Ready_RegisterSceneFactory())) return E_FAIL;

		/* Scene Ready */
		if (FAILED(Ready_Scene(SCENEID::TEST)))
			return E_FAIL;

		return S_OK;
	}

	void Update(float fTimeDelta)
	{
		if (g_pGameManager == nullptr)
			return;

		g_pGameManager->Pre_Update_Engine(fTimeDelta);
		g_pGameManager->Update_Engine(fTimeDelta);
		g_pGameManager->Late_Update_Engine(fTimeDelta);
		g_pGameManager->Add_RenderGroup();
	}

	HRESULT Render()
	{
		if (g_pGameManager == nullptr)
			return E_FAIL;

		g_pGameManager->GraphicDevice_RenderBegin(COLOR_BLACK);

		g_pGameManager->Draw();

		if (g_pfnBeforeEndFrame != nullptr)
			g_pfnBeforeEndFrame();

		return g_pGameManager->EndFrame();
	}

	void Release()
	{
		if (g_pGameManager == nullptr)
			return;

		g_pDevice.Reset();
		g_pContext.Reset();

		g_pGameManager->Release_Manager();
		Safe_Release(g_pGameManager);
	}

	bool Frame()
	{
		static float s_fTimeAcc = 0.f;
		if (g_pGameManager == nullptr)
			return false;

		/* 원본 Client 메인 루프 : Timer_Default 로 시간을 누적하고 1/60 초마다 한 프레임 */
		g_pGameManager->TimerManager_Compute_TimeDelta(L"Timer_Default");
		s_fTimeAcc += g_pGameManager->TimerManager_Get_TimeDelta(L"Timer_Default");

		if (s_fTimeAcc < 1.f / 60.f)
			return false;

		g_pGameManager->TimerManager_Compute_TimeDelta(L"Timer_60");
		const float fTimeDelta = g_pGameManager->TimerManager_Get_TimeDelta(L"Timer_60");
		Update((std::min)(fTimeDelta, 0.1f));
		Render();
		s_fTimeAcc = 0.f;
		return true;
	}

	uint32 Get_FPS()
	{
		return g_pGameManager != nullptr ? g_pGameManager->TimerManger_Get_FPS(L"Timer_60") : 0;
	}

	void Set_RenderCallback(void(*pfnBeforeEndFrame)())
	{
		g_pfnBeforeEndFrame = pfnBeforeEndFrame;
	}
}
