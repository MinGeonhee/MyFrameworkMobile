#pragma once

#include "Engine_Macro.h"
#include "CBase.h"
#include "Engine_Enum.h"

class CGameManager;

class CMainApp final : public CBase
{
private:
    CMainApp() = default;

public:
    ~CMainApp() override;

    HRESULT Initialize(HWND hWnd,
                       uint32 iWinSizeX,
                       uint32 iWinSizeY,
                       GRAPHIC_API eGraphicAPI);
    void Update();
    void Render();
    void Resize(uint32 iWinSizeX, uint32 iWinSizeY);

private:
    CGameManager* m_pGameManager = nullptr;

public:
    static CMainApp* Create(HWND hWnd,
                            uint32 iWinSizeX,
                            uint32 iWinSizeY,
                            GRAPHIC_API eGraphicAPI);
    void Free() override;
};
