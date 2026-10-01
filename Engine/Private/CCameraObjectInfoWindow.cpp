#include "CCameraObjectInfoWindow.h"
#include "CImGuiFuntion.h"
#include "CCameraComponent.h"
#include "EnumUtils.h"
#include "CTransform.h"
#include "Slider_Float.h"
#include "CUICameraObject.h"
#include "Button.h"
#include "EnumRadioButton.h"
#include "CDefaultCameraObject.h"

#ifdef _DEBUG

CCameraObjectInfoWindow::CCameraObjectInfoWindow(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CImGuiWindow(hWnd, pDevice, pContext), m_pTargetCameraObject(nullptr), m_vecTargetCameraObject()
    , m_tDefautlOffset{}, m_fDefaultFar{}, m_fDefaultNear{}, m_fDefaultFov{}
{
}

void CCameraObjectInfoWindow::Update()
{
	if (ImGui::Begin("CameraObject Window"))
		MainWindow();

	ImGui::End();

}


void CCameraObjectInfoWindow::MainWindow()
{
	CameraObjectInfoWindow();
}

void CCameraObjectInfoWindow::CameraObjectInfoWindow()
{
    /* GameObject 에서 Transform 받아오기? */

    if (m_pTargetCameraObject == nullptr)
    {
        CImGuiFuntion::ColorText(COLOR_GREEN, "Empty");
    }

    else
    {
		ImGui::Text("Selected Camera: %s", WString_To_String(m_pTargetCameraObject->GetName()).c_str());
		ImGui::Separator();
        m_pTargetCameraObject->ImGuiUpdate();
    }
}

CCameraObjectInfoWindow* CCameraObjectInfoWindow::Create(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CCameraObjectInfoWindow* pCameraObjectInfoWindow = new CCameraObjectInfoWindow(hWnd, pDevice, pContext);
    

    if (FAILED(pCameraObjectInfoWindow->CImGuiWindow::Initialize()))
    {
        
        (pCameraObjectInfoWindow);
        MSG_BOX("pCameraObjectInfoWindow is failed to Create");
        return nullptr;
    }

    return pCameraObjectInfoWindow;
}
#endif // DEBUG

