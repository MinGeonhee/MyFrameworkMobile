#pragma once
#include "CBase.h"
#include <functional>
class ENGINE_DLL CScreenShotManager final:public CBase
{
public:using CaptureHandler=std::function<HRESULT(const std::wstring&,bool)>;
private:CScreenShotManager()=default;
public:
    ~CScreenShotManager()override=default;
    void Request_ScreenShot(bool isOverlay,const std::wstring& filePath=L"");
    HRESULT ExecutePending_ScreenShot();
    void Set_CaptureHandler(CaptureHandler Handler){m_CaptureHandler=std::move(Handler);}
    bool Get_IsRequested()const{return m_isRequested;}
    static CScreenShotManager* Create();void Free()override;
private:bool m_isRequested=false,m_isOverlayRequested=false;std::wstring m_FilePath;CaptureHandler m_CaptureHandler;
};
