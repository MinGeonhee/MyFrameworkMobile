#include "CFile.h"
#include "CGameManager.h"
#include "CTexture.h"
#include "CModel.h"

CFile::CFile()
    : m_tInfo{} , m_pModel(nullptr) , m_pTexture(nullptr)
     , m_eFileType(FILE_TYPE::NONE)
{
}
CFile::~CFile()
{
}

HRESULT CFile::Initialize(const wstring& wstrPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    if (wstrPath.empty()) return E_FAIL;

    m_tInfo = FILE_INFO(wstrPath);

    /* 이미지 관련일떄... */
    if (m_tInfo.wstrFileEXT == L".png" || m_tInfo.wstrFileEXT == L".jpg" || m_tInfo.wstrFileEXT == L".jpeg"
        || m_tInfo.wstrFileEXT == L".bmp" || m_tInfo.wstrFileEXT == L".dds")
    {
        m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(m_tInfo.wstrFileNameEXT,RESOURCE_TYPE::TEXTURE,false));
        /* 파일이 없다면 */
        if (m_pTexture == nullptr)
        {
            m_pTexture = CTexture::Create(m_tInfo.wstrFileFullPath,1,pDevice,pContext);
            if (FAILED(CGameManager::GetInstance()->ResourceManager_Add_Resource(m_tInfo.wstrFileNameEXT, m_pTexture)))
            {
                Safe_Release(m_pTexture);
                return E_FAIL;
            }
        }
    }
    else if (m_tInfo.wstrFileEXT == L".json")
    {
        wstring FileName = m_tInfo.wstrFileName;
        /* json 확장자 명일때 */

        m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(L".json",RESOURCE_TYPE::TEXTURE,false));

        /* 모델 정보가 존재한다면 */ /* 내가 이 모델 정보를 뽑아 올때..그냥 단순모델로도 뽑고싶은데... */
        /* ,,,,흠 조진거같은데......어카지 */
        /* MapTool에서 Model을 뛰어서뭐 한다는게 잘모소딘거같기도하고 */
        if (FileName.find(L"_ModelInfo") != wstring::npos)
        {
            wstring FileName = m_tInfo.wstrFileName;
            wstring EraseKey = L"_ModelInfo";
            FileName = FileName.erase(FileName.find(EraseKey),EraseKey.length());
      
            m_pModel = static_cast<CModel*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(FileName, RESOURCE_TYPE::MODEL, true));

            if (m_pModel == nullptr)
            {

                CModel* pModel = CModel::Create(m_tInfo.wstrFileFullPath,XMMatrixIdentity(),pDevice,pContext);
                if (FAILED(CGameManager::GetInstance()->ResourceManager_Add_Resource(FileName, pModel)))
                {
                    Safe_Release(pModel);
                    return E_FAIL;
                }
                m_pModel = static_cast<CModel*>(pModel->Copy_Resource(nullptr));
            }

            m_eFileType = FILE_TYPE::MODEL;
            return S_OK;
        }

    }
    else
    {
        if (m_tInfo.wstrFileEXT == L".fbx")
            m_eFileType = FILE_TYPE::MODEL;

        m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(m_tInfo.wstrFileEXT,RESOURCE_TYPE::TEXTURE,false));
    }
    if (m_pTexture == nullptr)
    {
        wstring wstrEXT = L"NoImage";
        m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(wstrEXT, RESOURCE_TYPE::TEXTURE, false));
    }


    return S_OK;
}

CFile* CFile::Create(const wstring& wstrPath , ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CFile* pFile = new CFile();

    if (FAILED(pFile->Initialize(wstrPath,pDevice,pContext)))
    {
        Safe_Release(pFile);
        MSG_BOX("CFile is Failed to Create");
        return nullptr;
    }

    return pFile;
}

void CFile::Free()
{
    __super::Free();
    /* CFile::Free() */
    Safe_Release(m_pModel);
    Safe_Release(m_pTexture);
}
