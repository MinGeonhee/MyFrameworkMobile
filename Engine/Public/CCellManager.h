#pragma once
#include "CBase.h"

namespace Engine
{
	class CCell;
}


NS_BEGIN(Engine)

class CCellManager :public CBase
{
private:
	explicit CCellManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CCellManager() = default;

private:
	HRESULT				Initialize();
public:

	HRESULT					Add_Cells(const wstring& wstrCellPath, const wstring& AddKey);
	const vector<CCell*>&	Get_Cells(const wstring& wstrFindKey);

	bool					Get_IsCellRender() const { return m_isCellRender;}
	void					Set_IsCellRender(bool isRender) { m_isCellRender = isRender;}

private:
	static vector<CCell*>	m_vecCellDummy;
	/* Path의 이름값으로아마 저장될 예정이다 */
	map< wstring, vector<CCell*>> m_mapCells;

	bool					m_isCellRender;
private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	static	CCellManager*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void			Free() override;
};

NS_END