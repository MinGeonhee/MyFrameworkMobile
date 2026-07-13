#pragma once

#include "CBase.h"
#include "Engine_Struct.h"

class CGraphicDevice;
class CTimeManger;

class ENGINE_DLL CGameManager final : public CBase
{
    DECLARE_SINGLETON(CGameManager)

private:
    CGameManager();
    ~CGameManager() override = default;

public:
    HRESULT Initialize_Engine(const Engine_Desc& tEngineDesc);

    void Pre_Update_Engine(float fTimeDelta);
    void Update_Engine(float fTimeDelta);
    void Late_Update_Engine(float fTimeDelta);
    void Draw();
    void Resize_Engine(uint32 iWinSizeX, uint32 iWinSizeY);
    void Release_Engine();

#pragma region TimerManager
    HRESULT TimerManager_Add_Timer(const std::wstring& strKey);
    float TimerManager_Get_TimeDelta(const std::wstring& strKey) const;
    void TimerManager_Compute_TimeDelta(const std::wstring& strKey);
    uint32 TimerManger_Get_FPS(const std::wstring& strKey) const;
#pragma endregion

private:
    CGraphicDevice* m_pGraphicDevice = nullptr;
    CTimeManger* m_pTimerManager = nullptr;

private:
    void Free() override;
};
