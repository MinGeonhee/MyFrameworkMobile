#include "CCellManager.h"
#include "CCell.h"

vector<CCell*> CCellManager::m_vecCellDummy{};

CCellManager::CCellManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_mapCells{} , m_isCellRender(false)
{
}

HRESULT CCellManager::Initialize()
{
    return S_OK;
}

/* 추가할 Cell 번호이다. */
HRESULT CCellManager::Add_Cells(const wstring& wstrCellPath, const wstring& AddKey)
{
    const vector<CCell*>& pFindCell = Get_Cells(AddKey);
    if(!pFindCell.empty()) return E_FAIL;


    CFileStream fs(wstrCellPath, std::ios::binary);
    if (!fs.is_open()) return E_FAIL;

    /* Cell에 대한 정점 정보를 가져온다 */

    Cell_Info tInfo{};

    while (true)
    {
        fs.read(reinterpret_cast<char*>(&tInfo), sizeof(CELL_INFO));

        if (fs.eof())break;

        CCell* pCell = CCell::Create(tInfo, m_pDevice, m_pDeviceContext);
        NULLPTR_RETURN_EFAIL(pCell);

        m_mapCells[AddKey].push_back(pCell);
    }

    return S_OK;
}

const vector<CCell*>& CCellManager::Get_Cells(const wstring& wstrFindKey)
{
    auto iter = m_mapCells.find(wstrFindKey);

    if (iter == m_mapCells.end()) return m_vecCellDummy;

    return iter->second;
}

CCellManager* CCellManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CCellManager* pCellManager = new CCellManager(pDevice, pContext);
    if(FAILED(pCellManager->Initialize()))
    {
        Safe_Release(pCellManager);
        MSG_BOX("Cell Manager is failed to Create");
        return nullptr;
    }

    return pCellManager;
}

void CCellManager::Free()
{
    for (auto& Cells : m_mapCells)
        for (auto& Cell : Cells.second)
            Safe_Release(Cell);
}
