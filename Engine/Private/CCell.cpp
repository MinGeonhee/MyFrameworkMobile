#include "CCell.h"
#include "CVIBuffer_Cell.h"

CCell::CCell(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_iIndex(0), m_iNeighborIndex{ -1,-1,-1 }, m_vNormal{}, m_vPosition{}, m_vPlane{}
    , m_iCellType{ 0 }, m_isActive{ true }, m_vCenterPos{0.f,0.f,0.f}
#ifdef _DEBUG
    ,m_pBuffer{nullptr}
#endif // _DEBUG


{
}

CCell::~CCell()
{
}

HRESULT CCell::Initialize(Float3* pPosition[Cast_Uint(CELL_POINT::END)], int32 iCellIndex)
{
    /* Cell Index */
    m_iIndex = iCellIndex;

    /* Position 복사 */
    memcpy(&m_vPosition,pPosition,sizeof(Float3) * Cast_Uint(CELL_POINT::END));

    /* 법선 벡터 구하기 */
    Vector4 vLine{};
    /* AB */
    vLine =  XMVector3Normalize(XMVectorSetY(XMVectorSubtract(XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::B)]), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::A)])),0.f));
    XMStoreFloat3(&m_vNormal[Cast_Uint(CELL_LINE::AB)], XMVectorSet(XMVectorGetZ(vLine) * -1.f , 0.f, XMVectorGetX(vLine),1.f));
    /* BC */
    vLine = XMVector3Normalize(XMVectorSetY(XMVectorSubtract(XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::C)]), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::B)])), 0.f));
    XMStoreFloat3(&m_vNormal[Cast_Uint(CELL_LINE::BC)], XMVectorSet(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine), 1.f));
    /* CA */
    vLine = XMVector3Normalize(XMVectorSetY(XMVectorSubtract(XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::A)]), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::C)])), 0.f));
    XMStoreFloat3(&m_vNormal[Cast_Uint(CELL_LINE::CA)], XMVectorSet(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine), 1.f));


    if (FAILED(Ready_Component()))
        return E_FAIL;

    return S_OK;
}

HRESULT CCell::Load_Initialize(const CELL_INFO& tLoadInfo)
{

    memcpy(&m_vPosition,&tLoadInfo.vPosition,sizeof(Float3) * Cast_Uint(CELL_POINT::END));
    memcpy(&m_vNormal,&tLoadInfo.vNormal,sizeof(Float3) * Cast_Uint(CELL_POINT::END));
    memcpy(&m_vPlane,&tLoadInfo.vPlane,sizeof(Float4));
    memcpy(&m_iNeighborIndex, &tLoadInfo.iNeighborIndex, sizeof(int32) * Cast_Uint(CELL_LINE::END));

    m_vCenterPos = tLoadInfo.vCenterPos;

    m_iIndex =  tLoadInfo.iIndex;
    m_iCellType = tLoadInfo.iCellType;
    m_isActive = tLoadInfo.isActive;

    if (FAILED(Ready_Component()))
        return E_FAIL;

    return S_OK;
}

HRESULT CCell::Ready_Component()
{
#ifdef  _DEBUG
    CVIBuffer_Cell* pBuffer = CVIBuffer_Cell::Create(m_vPosition,m_pDevice, m_pDeviceContext);
    if (!pBuffer) return E_FAIL;
    m_pBuffer = pBuffer;
#endif //  _DEBUG
    return S_OK;
}




HRESULT CCell::SetUp_NeighborCell(CCell* pCheckCell)
{
    NULLPTR_RETURN_EFAIL(pCheckCell);

    if (this == pCheckCell) return S_OK;

    /* pCell과 비교를 시작 */
    if (pCheckCell->CompareCell(&this->m_vPosition[Cast_Uint(CELL_POINT::A)], &this->m_vPosition[Cast_Uint(CELL_POINT::B)]))
        this->m_iNeighborIndex[Cast_Uint(CELL_LINE::AB)] = pCheckCell->m_iIndex;

    if (pCheckCell->CompareCell(&this->m_vPosition[Cast_Uint(CELL_POINT::B)], &this->m_vPosition[Cast_Uint(CELL_POINT::C)]))
        this->m_iNeighborIndex[Cast_Uint(CELL_LINE::BC)] = pCheckCell->m_iIndex;

    if (pCheckCell->CompareCell(&this->m_vPosition[Cast_Uint(CELL_POINT::C)], &this->m_vPosition[Cast_Uint(CELL_POINT::A)]))
        this->m_iNeighborIndex[Cast_Uint(CELL_LINE::CA)] = pCheckCell->m_iIndex;

    return S_OK;
}



bool CCell::CompareCell(const Float3* pSour, const Float3* pDest)
{
    Vector4 EpslionVector = XMVectorSet(EPSILON, EPSILON, EPSILON, EPSILON);

    if (XMVector3NearEqual(XMLoadFloat3(pSour), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::A)]), EpslionVector))
    {
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::B)]), EpslionVector))
            return true;
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::C)]), EpslionVector))
            return true;
    }
    if (XMVector3NearEqual(XMLoadFloat3(pSour), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::B)]), EpslionVector))
    {
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::C)]), EpslionVector))
            return true;
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::A)]), EpslionVector))
            return true;
    }
    if (XMVector3NearEqual(XMLoadFloat3(pSour), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::C)]), EpslionVector))
    {
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::A)]), EpslionVector))
            return true;
        if (XMVector3NearEqual(XMLoadFloat3(pDest), XMLoadFloat3(&m_vPosition[Cast_Uint(CELL_POINT::B)]), EpslionVector))
            return true;
    }

    return false;
}

#ifdef _DEBUG
HRESULT	CCell::Render()
{
    //if (FAILED(m_pBuffer->Bind_Resource()))
    //    return E_FAIL;
    //
    //if (FAILED(m_pBuffer->Draw()))
    //    return E_FAIL;

    return S_OK;
}
#endif

bool CCell::IsIn( const fVector4_1& vCellSpacePos, int32& iNeighborIndex)
{

    for (uint32 i = 0; i < Cast_Uint(CELL_LINE::END); ++i)
    {
        /* 방향벡터 */
        Vector4 Dir = XMVector3Normalize(vCellSpacePos - XMLoadFloat3(&m_vPosition[i]));
        
        /* 삼가형의 한선분이랑 내적한 값이 0 보다 크면벗어난거 */
        if (Vector3Dot(Dir, XMLoadFloat3(&m_vNormal[i])) > 0)
        {
            iNeighborIndex = m_iNeighborIndex[i];
            return false;
        }
    }

    return true;
}

/* 이건 내 이웃이 == -1 일떄만 상요할 노말 */
bool CCell::IsIn(const fVector4_1& vCellSpacePos, Float3* vOutNormal, int32& iNeighborIndex)
{

    for (uint32 i = 0; i < Cast_Uint(CELL_LINE::END); ++i)
    {
        /* 방향벡터 */
        Vector4 Dir = XMVector3Normalize(vCellSpacePos - XMLoadFloat3(&m_vPosition[i]));
        *vOutNormal = m_vNormal[i];

        /* 삼가형의 한선분이랑 내적한 값이 0 보다 크면벗어난거 */
        if (Vector3Dot(Dir, XMLoadFloat3(&m_vNormal[i])) > 0)
        {
            iNeighborIndex = m_iNeighborIndex[i];
            return false;
        }
    }

    return true;
}



bool CCell::Comput_IsInCell(const fVector4_1& vPos)
{
    for (uint32 i = 0; i < Cast_Uint(CELL_LINE::END); ++i)
    {
        /* 방향벡터 */
        Vector4 Dir = XMVector3Normalize(vPos - XMLoadFloat3(&m_vPosition[i]));

        /* 삼가형의 한선분이랑 내적한 값이 0 보다 크면벗어난거 */
        if (Vector3Dot(Dir, XMLoadFloat3(&m_vNormal[i])) > 0)
            return false;
    }

    return true;
}



float CCell::Compute_Height(const fVector4_1& vPos)
{
    return -((XMVectorGetX(vPos) * m_vPlane.x + XMVectorGetZ(vPos) * m_vPlane.z + m_vPlane.w ) / m_vPlane.y);
}




CCell* CCell::Create(Float3* pPosition[Cast_Uint(CELL_POINT::END)], int32 iCellIndex ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CCell* pCell = new CCell(pDevice, pContext);

    if (FAILED(pCell->Initialize(pPosition, iCellIndex)))
    {
        Safe_Release(pCell);
        MSG_BOX("Cell is failed to Create");
        return nullptr;
    }

    return pCell;
}

CCell* CCell::Create(const CELL_INFO& tLoadInfo, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CCell* pCell = new CCell(pDevice, pContext);

    if (FAILED(pCell->Load_Initialize(tLoadInfo)))
    {
        Safe_Release(pCell);
        MSG_BOX("Cell is failed to Create");
        return nullptr;
    }
    return pCell;
}

void CCell::Free()
{
    __super::Free();

    /* CCell::Free() */
#ifdef _DEBUG
    Safe_Release(m_pBuffer);
#endif // _DEBUG

}
