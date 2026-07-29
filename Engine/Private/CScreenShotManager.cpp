#include "pch.h"
#include "CScreenShotManager.h"
CScreenShotManager* CScreenShotManager::Create(){return new CScreenShotManager{};}
void CScreenShotManager::Request_ScreenShot(bool overlay,const std::wstring& path){m_isRequested=true;m_isOverlayRequested=overlay;m_FilePath=path;}
HRESULT CScreenShotManager::ExecutePending_ScreenShot()
{
    if(!m_isRequested)return S_FALSE;
    // Diligent 텍스처 Readback 구현은 플랫폼별 백엔드로 연결하며 요청 큐 인터페이스는 유지한다.
    const HRESULT result=m_CaptureHandler?m_CaptureHandler(m_FilePath,m_isOverlayRequested):E_NOTIMPL;
    m_isRequested=false;m_isOverlayRequested=false;m_FilePath.clear();return result;
}
void CScreenShotManager::Free(){m_isRequested=false;m_FilePath.clear();m_CaptureHandler={};CBase::Free();}
