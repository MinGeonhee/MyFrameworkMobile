#include "CFloder.h"
#include "CGameManager.h"
#include "CFile.h"
#include "MyImGuiHeader.h"

CFloder::CFloder()
    : m_tInfo{} , m_pGameManager(CGameManager::GetInstance())
    ,m_pParentFloder(nullptr)
{
    Safe_AddRef(m_pGameManager);
}

HRESULT CFloder::Initialize(CFloder* pParentFloder,const wstring& wstrRootPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    
    /* 파일 이름이 있으면 FileName을 제거한다 */
    wstring wstr = String_To_WString(NormalizePath(filesystem::canonical(wstrRootPath)));

    m_tInfo.wstrFloderFullPath = String_To_WString(NormalizePath(filesystem::canonical(wstrRootPath))).c_str();
    m_tInfo.wstrFloderName = path(wstrRootPath).filename();

    /* ParentFloder Setting 나를 가지고있는 폴더의 포인터*/
    /* 약한참조 레퍼런스 증가 x */
    if (pParentFloder == nullptr)
        m_pParentFloder = pParentFloder;

    for (auto& Path : filesystem::directory_iterator(m_tInfo.wstrFloderFullPath))
    {
        /* 현재 폴더를 도는데 아마 typde 이 폴더라면? 돌아보자.. */
        path CurPath = NormalizePath (Path.path());


        /*  폴더일때 */
        if (!CurPath.has_extension())
        {
            ++m_tInfo.iCountFloders;
            CFloder* pFloder = CFloder::Create(this , wstring(CurPath.wstring()), pDevice,pContext);
            NULLPTR_RETURN_EFAIL(pFloder);
            m_mapTreeFloder.emplace( pFloder->m_tInfo.wstrFloderName , pFloder );
        }

        /* 파일 일때 */

        else
        {
            ++m_tInfo.iCountFiles;
            CFile* pFile = CFile::Create(CurPath, pDevice,pContext);
            NULLPTR_RETURN_EFAIL(pFile)
            m_vecFile.push_back(pFile);
        }
    }

    path Path(wstrRootPath);

    return S_OK;
}


CFloder* CFloder::Create(CFloder* pParentFloder,const wstring& wstrRootPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CFloder* pFloder = new CFloder;

    if (FAILED(pFloder->Initialize(pParentFloder, wstrRootPath, pDevice,pContext)))
    {
        Safe_Release(pFloder);
        MSG_BOX("FLoder is Faield to Create");
        return nullptr;
    }
    return pFloder;
}

void CFloder::Free()
{
    __super::Free();

    /* CRootFloder::Free() */

    /* 파일정리 */
    for (auto& CFile : m_vecFile)
        Safe_Release(CFile);
    /* 폴더 정리 */
    for (auto& Floder : m_mapTreeFloder)
        Safe_Release(Floder.second);


    Safe_Release(m_pGameManager);
    Safe_Release(m_pParentFloder);
}



#ifdef _DEBUG
void CFloder::ImGui_Update()
{

}

#endif // _DEBUG