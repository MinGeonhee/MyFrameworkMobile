#pragma once
#include "CUIComponent.h"

namespace Engine
{
	class CCanvas;
}

NS_BEGIN(Engine)

class ENGINE_DLL CUITransform final : public CUIComponent
{
public:
	typedef struct UITransform_Desc
	{
		Float2  vSize			{0.f,0.f};				/* 내 UI의 크기 */
		Float2	vPosition		{0.f,0.f};				/* 내 Local상의 위치값 */
		Float2  vAnchor			{0.5f,0.5f};			/* 부모 UI 기준한 내 위치 비율 */

		CUITransform*			pParentUITransform{ nullptr };
	}UITRANSFORM_DESC;
private:
	explicit					CUITransform(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit					CUITransform(const CUITransform& Proto);
	virtual						~CUITransform() {}
private:
	HRESULT						Proto_Initialize();
	virtual HRESULT				Clone_Initialize(void* pArg);
public:
	Vector4						Get_State_ToCal(STATE eState)const;
	Float4						Get_State_ToStore(STATE eState)const;
	void						Set_State(STATE eState, const Float4& vFloat4);
	void						Set_State(STATE eState, const fVector4_1& vVector4);
public:
	void						Update_World(const CCanvas* pCanvas);
	void						Update_World_Font(const CCanvas* pCanvas);
	const Float4x4*				Get_WorldPtr() { return &m_matWorld; }
	const Float4x4&				Get_World()    { return m_matWorld; }
public:
	void						Set_IsMoveEvent(bool isMove) { m_isMoveEvent = isMove; }
	void						Set_ParentUITransform(CUITransform* pParentUITransform) { m_pParentUITransform = pParentUITransform; }
	void						Set_Anchor(const Float2& vAnchor)						{ m_vAnchor = vAnchor; }
	void						Set_Pos(const Float2& vPos)								{ m_vPosition = vPos; }
	void						Set_Size(const Float2& vSize)							{ m_vSize = vSize;}
public:	
	const Float2&				Get_FirstPosition()			const { return m_vFirstPosition;}
	const Float2&				Get_FirstSize()				const { return m_vFirstSize;}
	const Float2&				Get_FirstAnchor()			const { return m_vFirstAnchor;}
public:
	bool						Get_IsMoveEvent()			const { return m_isMoveEvent; }
	CUITransform*				Get_ParentUITransform()		const { return m_pParentUITransform; }		
	const Float2&				Get_Anchor()				const { return m_vAnchor; }
	const Float2&				Get_Pos()					const { return m_vPosition; }
	const Float2&				Get_Size()					const { return m_vSize; }
public:
	bool						Move(const fVector4_1& vDir , float DT);
private:

	Float4x4					m_matLocal{};
	Float4x4					m_matWorld{};

	Float2						m_vFirstSize;
	Float2						m_vFirstPosition;
	Float2						m_vFirstAnchor;


	Float2						m_vSize;
	Float2						m_vPosition;
	Float2						m_vAnchor;


	bool						m_isChange;

	CUITransform*				m_pParentUITransform;

	bool						m_isMoveEvent;

public:
	static CUITransform*   Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CComponent*	   Clone(void* pArg)override;
	virtual void		   Free()			override;
};

NS_END
