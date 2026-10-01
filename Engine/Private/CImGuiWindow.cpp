#include "CImGuiWindow.h"
#ifdef _DEBUG



CImGuiWindow::CImGuiWindow( HWND hWnd,ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext)
	: m_hWnd(hWnd), m_pDevice(pDevice), m_pDeviceContext(pDeviceContext)
{

}


HRESULT	CImGuiWindow::Initialize()
{


    return S_OK;
}

void CImGuiWindow::Free()
{
    __super::Free(); 
  

}

#endif // _DEBUG
