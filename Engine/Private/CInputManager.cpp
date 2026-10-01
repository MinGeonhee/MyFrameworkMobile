#include "CInputManager.h"
#include "CGameManager.h"
#include "CCameraObjectBase.h"
#include "CCalulator.h"
#include "CGraphicDevice.h"

#if defined(_WIN32)
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#endif

CInputManager::CInputManager(HINSTANCE hInst, HWND hwnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext, uint32 iWinSizeX, uint32 iWinSizeY)
	: m_bKeyLock(false), m_iWinSizeX(iWinSizeX), m_iWinSizeY(iWinSizeY), m_tMousePos{},
	m_hWnd(hwnd), m_tMouseState(), m_vecStates{}
	, m_hInstance{ hInst }, m_pDevice(pDevice), m_pDeviceContext(pDeviceContext), m_pGameManager(CGameManager::GetInstance()), m_tPreFrameMoustPos{}
{
	Safe_AddRef(m_pGameManager);
}

HRESULT CInputManager::Initialize()
{
	m_vecStates.resize(KEY_TYPE_COUNT);

#if defined(_WIN32)
	// DInput 컴객체를 생성하는 함수
	if (FAILED(DirectInput8Create(static_cast<HINSTANCE>(m_hInstance),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pInputSDK,
		NULL)))
		return E_FAIL;

	// 마우스 객체 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;

	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	m_pMouse->SetDataFormat(&c_dfDIMouse);

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	m_pMouse->SetCooperativeLevel(static_cast<HWND>(m_hWnd), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	m_pMouse->Acquire();
#endif

	return S_OK;
}

void CInputManager::Update(float DT)
{
	BYTE asciikeys[KEY_TYPE_COUNT] = {};

#if defined(_WIN32)
	HWND hwnd = ::GetActiveWindow();
	if (m_hWnd != hwnd)
	{
		for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
			m_vecStates[key] = KEY_STATE::END;
		return;
	}

	if (::GetKeyboardState(asciikeys) == false)
		return;
#endif

	/* 가상 키 / 터치 입력 (Android , 테스트용) */
	{
		std::lock_guard<std::mutex> Lock(m_NativeMutex);
		for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
			if (m_isNativeKey[key])
				asciikeys[key] |= 0x80;
		if (m_tNativeTouch[0].isPressed)
			asciikeys[VK_LBUTTON] |= 0x80;
		if (m_tNativeTouch[1].isPressed)
			asciikeys[VK_RBUTTON] |= 0x80;
	}

	for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
	{
		if (asciikeys[key] & 0x80)
		{
			KEY_STATE& state = m_vecStates[key];

			// 이전 프레임에 키를 누른 상태라면 PRESS
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::PRESS;
			else
				state = KEY_STATE::DOWN;
		}
		else
		{
			KEY_STATE& state = m_vecStates[key];

			// 이전 프레임에 키를 누른 상태라면 UP
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::UP;
			else
				state = KEY_STATE::END;
		}
	}

	/* 업데이트되기 전 마우스 좌표를 이전프레임 마우스좌표에 넣어준다 */
	m_tPreFrameMoustPos = m_tMousePos;

#if defined(_WIN32)
	m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);

	/* 그 후 업데이트 */
	GetCursorPos(&m_tMousePos);
	ScreenToClient(static_cast<HWND>(m_hWnd), &m_tMousePos);
#else
	/* 터치 → 마우스 : 슬롯 1(화면 오른쪽) 은 시점 회전(RBUTTON 드래그) , 슬롯 0(왼쪽) 은 LBUTTON / 마우스 위치 */
	std::lock_guard<std::mutex> Lock(m_NativeMutex);
	ZeroMemory(&m_tMouseState, sizeof(m_tMouseState));
	const NATIVE_TOUCH& tLook = m_tNativeTouch[1].isPressed ? m_tNativeTouch[1] : m_tNativeTouch[0];
	const bool isAnyTouch = m_tNativeTouch[0].isPressed || m_tNativeTouch[1].isPressed;
	if (isAnyTouch)
	{
		POINT tPos{ static_cast<LONG>(tLook.fX), static_cast<LONG>(tLook.fY) };
		if (m_isPrevTouch)
		{
			m_tMouseState.lX = tPos.x - m_tPrevTouchPos.x;
			m_tMouseState.lY = tPos.y - m_tPrevTouchPos.y;
		}
		m_tPrevTouchPos = tPos;
	}
	if (m_tNativeTouch[0].isPressed)
	{
		m_tMouseState.rgbButtons[DIM_LB] = 0x80;
		m_tMousePos = POINT{ static_cast<LONG>(m_tNativeTouch[0].fX), static_cast<LONG>(m_tNativeTouch[0].fY) };
	}
	if (m_tNativeTouch[1].isPressed)
		m_tMouseState.rgbButtons[DIM_RB] = 0x80;
	/* 시점 손가락이 바뀌면 이동량이 튀지 않도록 다시 시작한다 */
	static bool s_isPrevLookSlot1 = false;
	if (s_isPrevLookSlot1 != m_tNativeTouch[1].isPressed)
		m_tMouseState.lX = m_tMouseState.lY = 0;
	s_isPrevLookSlot1 = m_tNativeTouch[1].isPressed;
	m_isPrevTouch = isAnyTouch;
#endif
}

void CInputManager::Set_NativeTouch(uint32 iSlot, bool isPressed, float fScreenX, float fScreenY)
{
	if (iSlot >= 2)
		return;

	/* SwapChain 좌표 → 엔진 화면 좌표 (엔진은 고정 해상도를 화면비 유지로 확대해서 출력한다) */
	float fX = fScreenX, fY = fScreenY;
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
	{
		if (Diligent::ISwapChain* pSwapChain = pGraphicDevice->Get_SwapChain())
		{
			const Diligent::SwapChainDesc& tDesc = pSwapChain->GetDesc();
			const float fScale = (std::min)(tDesc.Width / static_cast<float>(m_iWinSizeX), tDesc.Height / static_cast<float>(m_iWinSizeY));
			const float fOffsetX = (tDesc.Width - m_iWinSizeX * fScale) * 0.5f;
			const float fOffsetY = (tDesc.Height - m_iWinSizeY * fScale) * 0.5f;
			if (fScale > 0.f)
			{
				fX = (fScreenX - fOffsetX) / fScale;
				fY = (fScreenY - fOffsetY) / fScale;
			}
		}
	}

	std::lock_guard<std::mutex> Lock(m_NativeMutex);
	m_tNativeTouch[iSlot].isPressed = isPressed;
	m_tNativeTouch[iSlot].fX = fX;
	m_tNativeTouch[iSlot].fY = fY;
}

void CInputManager::Set_NativeKey(KEY_TYPE eKey, bool isPressed)
{
	std::lock_guard<std::mutex> Lock(m_NativeMutex);
	m_isNativeKey[static_cast<uint8>(eKey)] = isPressed;
}

void CInputManager::Mouse_Fix()
{
	POINT ptMouse{ (LONG)m_iWinSizeX >> 1, (LONG)m_iWinSizeY >> 1 };

	ClientToScreen(m_hWnd, &ptMouse);
	SetCursorPos(ptMouse.x, ptMouse.y);
}

void CInputManager::Mouse_Fix(uint32 iStartX, uint32 iStartY, uint32 iWinSizeX, uint32 iWinSizeY)
{
	POINT ptMouse{ ((LONG)iWinSizeX >> 1) + (LONG)iStartX , ((LONG)iWinSizeY >> 1) + (LONG)iStartY };

	ClientToScreen(m_hWnd, &ptMouse);
	SetCursorPos(ptMouse.x, ptMouse.y);
}

void CInputManager::Mouse_Set_Show(bool isShow)
{
#if defined(_WIN32)
	ShowCursor(isShow);
#endif
}



RAY_INFO CInputManager::Get_LayWorld(const wstring& wstrCamName, UPDATE_MODE eUdateMode , RAY_TYPE eRayType)
{
	//tRay.vRayOrigin = { m_tMousePos.x , m_tMousePos.y, 0.f};

	VIEWPORT_DESC tViewPort = m_pGameManager->CameraManager_Get_Camera(wstrCamName, eUdateMode)->Get_CamViewPort();
	Float2 vNDC_Mouse = CCalulator::Get_ScreenPos_To_NDC_ToStore( static_cast<float>(m_tMousePos.x) ,static_cast<float>(m_tMousePos.y) , tViewPort.TopLeftX, tViewPort.TopLeftY,
																									tViewPort.Width, tViewPort.Height);

	/* 이렇게되면 Near Far로 설정된다.. 0.1f ~ 1000.f */
	Vector4 NDC_RayNear = XMVectorSet(vNDC_Mouse.x , vNDC_Mouse.y , 0.f ,1.f);
	Vector4 NDC_RayFar  = XMVectorSet(vNDC_Mouse.x , vNDC_Mouse.y , 1.f ,1.f);

	/* 역행렬 미리구해주기 */
	Float4x4 matInverseProj = m_pGameManager->PipeLine_Get_PipeLineMatrix(wstrCamName, D3DTS::PROJ, true);
	Float4x4 matInverseView = m_pGameManager->PipeLine_Get_PipeLineMatrix(wstrCamName, D3DTS::VIEW, true);

	/* 뷰스페이스 공간 마우스 좌표 얻었다 */
	Vector4 View_RayNear = XMVector4Transform(NDC_RayNear,XMLoadFloat4x4(&matInverseProj));
	Vector4 View_RayFar = XMVector4Transform(NDC_RayFar,XMLoadFloat4x4(&matInverseProj));
	/* w나누기까지 */
	View_RayNear	/= XMVectorGetW(View_RayNear);
	View_RayFar /= XMVectorGetW(View_RayFar);
	//XMVector3TransformCoord()		자동     으로 w로 나눔	좌표(위치) 변환용
	//	XMVector3TransformNormal()	안 함	법선 변환용(w = 0 취급)
	//	XMVector4Transform()		안 함	 4D 벡터 그대로 곱(직접 w 나눠야 함)

	/* 월드 공간 마우스 좌표 */
	Vector4 WorldNearRay = XMVector4Transform(View_RayNear, XMLoadFloat4x4(&matInverseView));
	Vector4 WorldFarRay   = XMVector4Transform(View_RayFar, XMLoadFloat4x4(&matInverseView));

	Float4 vCamPos = m_pGameManager->PipeLine_Get_CameraPosition(wstrCamName);

	RAY_INFO tRayInfo{};
	tRayInfo.vRayOrigin = { XMVectorGetX(WorldNearRay) ,XMVectorGetY(WorldNearRay), XMVectorGetZ(WorldNearRay) };
	XMStoreFloat3(&tRayInfo.vRayDir , XMVector3Normalize(WorldFarRay - WorldNearRay));
	tRayInfo.eRayType = eRayType;


	return tRayInfo;
}


CInputManager* CInputManager::Create(HINSTANCE hInst, HWND hwnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext, uint32 iWinSizeX, uint32 iWinSizeY)
{
	CInputManager* pInputManager = new CInputManager(hInst, hwnd, pDevice ,pDeviceContext,iWinSizeX, iWinSizeY);

	INIT_FAILED_CEHCK(pInputManager);

	return pInputManager;
}

void CInputManager::Free()
{
#if defined(_WIN32)
	if (m_pMouse != nullptr)
		m_pMouse->Unacquire();
	Safe_Release(m_pMouse);
	Safe_Release(m_pInputSDK);
#endif
	Safe_Release(m_pGameManager);
	m_vecStates.clear();
}
