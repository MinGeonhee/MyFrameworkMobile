#pragma once

#include "CBase.h"

#include <map>
#include <string>

class CTimer;

class CTimeManger final : public CBase
{
private:
    CTimeManger() = default;

public:
    ~CTimeManger() override;

    HRESULT Add_Timer(const std::wstring& strKey);
    float Get_TimeDelta(const std::wstring& strKey) const;
    uint32 Get_FPS(const std::wstring& strKey) const;
    void Compute_TimeDelta(const std::wstring& strKey);

private:
    HRESULT Initialize();
    CTimer* Find_Timer(const std::wstring& strKey) const;

private:
    std::map<std::wstring, CTimer*> m_mapTimer;

public:
    static CTimeManger* Create();
    void Free() override;
};
