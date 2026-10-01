#pragma once
#include "CComponent.h"

namespace Engine
{
	class CCell;
	class CShader;
	class CCameraObjectBase;
}


NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct NavigationCom_Desc
	{
		const Float4x4* pParentMatrix	{nullptr};
		wstring			wstrNavigationKey{};
		int32			iCurMyCellIndex{ FIELD_CELL_INDEX };
	}NAVIGATIONCOM_DESC;

public:
	HRESULT	 ChangeCells(const NavigationCom_Desc& tNavDesc);
	Vector4	 GetCellCenterPos(int32 iCellIndex);
	Vector4	 GetCurCellCenterPos();
	int32	 ComputeCellIndex(const fVector4_1& vPos);
private:
	explicit CNavigation(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CNavigation(const CNavigation& Proto);
	virtual ~CNavigation() {}
protected:
	HRESULT				Proto_Initialize();
	virtual HRESULT		Clone_Initialize(void* pArg)override;
	HRESULT				SetUp_NeighborIndex();
	virtual HRESULT		Render(CCameraObjectBase* pRenderCam)override;
public:
#ifdef _DEBUG
	HRESULT				Render();
#endif // _DEBUG
public:
	Vector4				On_Plane(const fVector4_1& vPos);
	bool				Is_Move(const fVector4_1& vResultPos);
	bool				Is_VailedPosition(const  fVector4_1& vPos);
	/* 내가 가고자 하는 방향을 넣어줘야할듯? ... */
	Vector4				Is_MoveSliding(const fVector4_1& vLookDir, const fVector4_2& vCurPos,float fMoveDelta);

	void				Set_ParentMatrix(const Float4x4* pParentMatrix);
	void				Set_NavigationKey(const wstring& wstrKey) { m_wstrKey = wstrKey; }

	const Float4x4*		Get_ParentMatrix() const {return m_pParentMatrix;}
	const wstring&		Get_NavigationKey() const { return m_wstrKey; }
	int32				Get_OwnerCellIndex()   const { return m_iOwnerCellIndex; }

public:
	void				Set_OwnerCellIndex(int32 iIndex) { m_iOwnerCellIndex = iIndex; }
	void				Set_CellActive(int32 iCellIndex,bool isActive);

private:
	wstring				m_wstrKey;
	const Float4x4*		m_pParentMatrix;
	vector<CCell*>		m_vecCell;
	uint32				m_iOwnerCellIndex;
private:
#ifdef _DEBUG
	CShader*			m_pShader;
public:
	bool				m_isRender;
#endif // _DEBUG
public:
	static	CNavigation* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CComponent*  Clone(void* pArg);
	virtual	void Free()override;
};


NS_END
