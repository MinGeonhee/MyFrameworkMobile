#pragma once
#include "CBase.h"


/* OnOff 기능이 있는 Cell을 타입 일므추천좀 */

namespace Engine
{
	class CVIBuffer_Cell;
}


NS_BEGIN(Engine)

class ENGINE_DLL CCell final : public CBase
{
private:
	explicit CCell(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CCell();
private:
	HRESULT					Initialize(Float3* pPosition[Cast_Uint(CELL_POINT::END)], int32 iCellIndex);
	HRESULT					Load_Initialize(const CELL_INFO& tLoadInfo);
	HRESULT					Ready_Component();
public:
	HRESULT					SetUp_NeighborCell(CCell* pCheckCell);
	bool					CompareCell(const Float3* pSour, const Float3* pDest);
public:
	Vector4					Get_CenterPos() const { return XMLoadFloat3(&m_vCenterPos); }
	uint32					Get_Index() const { return m_iIndex; }
	bool					Get_IsActive() const { return m_isActive; }
public:
	void					Set_Index(uint32 iIndex) { m_iIndex = iIndex;}
	void					Set_IsActive(bool isActive) { m_isActive = isActive; }
public:

	bool					IsIn(const fVector4_1& vCellSpacePos,int32& iNeighborIndex);
	bool					IsIn(const fVector4_1& vCellSpacePos , Float3* vOutNormal ,int32& iNeighborIndex);
	float					Compute_Height(const fVector4_1& vPos);
	bool					Comput_IsInCell(const fVector4_1& vPos);


public:
#ifdef _DEBUG
	CVIBuffer_Cell*			m_pBuffer;
	HRESULT					Render();
#endif // _DEBUG
private:
	Float3					m_vPosition[Cast_Uint(CELL_POINT::END)];
	Float3					m_vCenterPos;
	Float3					m_vNormal[Cast_Uint(CELL_LINE::END)];
	Float4					m_vPlane;
	int32					m_iNeighborIndex[Cast_Uint(CELL_LINE::END)];
	int32					m_iIndex;
	uint32					m_iCellType;


	bool					m_isActive;
private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	static	CCell* Create( Float3* pPosition[Cast_Uint(CELL_POINT::END)], int32 iCellIndex, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	static  CCell* Create(const CELL_INFO& tLoadInfo, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free()override;
};

NS_END

