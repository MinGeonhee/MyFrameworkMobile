#pragma once
#include "CBase.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class ENGINE_DLL CLoader final:public CBase
{
public:
    using LOAD_FUNCTION=std::function<HRESULT()>;
private:
    struct LOAD_TASK{std::wstring strSubject;LOAD_FUNCTION Function;float fWeight=1.f;};
    CLoader()=default;~CLoader()override=default;
public:
    HRESULT Add_Task(const std::wstring& strSubject,LOAD_FUNCTION Function,float fWeight=1.f);
    HRESULT Start();void Wait();
    float Get_CurProgress()const;
    bool GetIsFinish()const{return m_isFinished.load();}
    bool GetIsFailed()const{return m_isFailed.load();}
    std::wstring Get_CurrentSubject()const;
    static CLoader* Create();void Free()override;
private:
    void Loading();
    std::vector<LOAD_TASK>m_Tasks;std::thread m_Worker;mutable std::mutex m_Mutex;
    std::wstring m_strCurrentSubject;float m_fTotalWeight=0.f,m_fCompletedWeight=0.f;
    std::atomic<bool>m_isStarted{false},m_isFinished{false},m_isFailed{false};
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
