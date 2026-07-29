#include "pch.h"
#include "CLoader.h"
CLoader* CLoader::Create(){return new CLoader{};}
HRESULT CLoader::Add_Task(const std::wstring&s,LOAD_FUNCTION f,float w){if(m_isStarted||s.empty()||!f||w<=0.f)return E_INVALIDARG;m_Tasks.push_back({s,std::move(f),w});m_fTotalWeight+=w;return S_OK;}
HRESULT CLoader::Start(){if(m_isStarted.exchange(true)||m_Tasks.empty())return E_FAIL;m_Worker=std::thread{&CLoader::Loading,this};return S_OK;}
void CLoader::Loading()
{
    // 워커에서는 파일 읽기/파싱 같은 CPU 작업만 수행한다. GPU 업로드와 Scene 변경은 메인 스레드가 담당한다.
    for(auto&t:m_Tasks){ {std::lock_guard<std::mutex>l{m_Mutex};m_strCurrentSubject=t.strSubject;} HRESULT hr=E_FAIL;try{hr=t.Function();}catch(...){hr=E_FAIL;}if(FAILED(hr)){m_isFailed=true;break;}{std::lock_guard<std::mutex>l{m_Mutex};m_fCompletedWeight+=t.fWeight;} }
    m_isFinished=true;
}
void CLoader::Wait(){if(m_Worker.joinable())m_Worker.join();}
float CLoader::Get_CurProgress()const{std::lock_guard<std::mutex>l{m_Mutex};return m_fTotalWeight>0.f?std::clamp(m_fCompletedWeight/m_fTotalWeight,0.f,1.f):0.f;}
std::wstring CLoader::Get_CurrentSubject()const{std::lock_guard<std::mutex>l{m_Mutex};return m_strCurrentSubject;}
void CLoader::Free(){Wait();m_Tasks.clear();m_strCurrentSubject.clear();CBase::Free();}
