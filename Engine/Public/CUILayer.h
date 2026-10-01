#pragma once
#include "CBase.h"

namespace Engine
{
	class CCanvas;
	class CUIGroup;
	class CUIObject;
}

NS_BEGIN(Engine)

/* CUI_Container로 전직할듯? 아마 이대로 내려서 사용할 거 같기도 하고 */

class ENGINE_DLL CUILayer : public CBase
{
public:
	explicit CUILayer();
	virtual ~CUILayer() {}
private:
	HRESULT	Initialize(UILAYER_ORDER eOrder);
public:
	void						Pre_Update(CCanvas* pCanvas,float DT);
	void						Update(CCanvas* pCanvas,float DT);
	void						Late_Update(CCanvas* pCanvas,float DT);
	HRESULT						Render(CCanvas* pCanvas);
public:

	HRESULT						Add_UIObject(CUIObject* pUIObject);
	CUIObject*					Get_UIObject(const wstring& wstrUIObjectName);

public:
	void						Set_UILayerDepth(UILAYER_ORDER eOrder)    { m_eUILayerOrder = eOrder; }
	void						Set_IsActive(bool isActive)	  { m_isActive = isActive; }
public:
	UILAYER_ORDER				Get_UILayerOrder()			const { return m_eUILayerOrder; }
	const vector<CUIObject*>&	Get_UIObjects()				const { return m_vecUIObject; }
	bool						Get_IsActive()				const { return m_isActive; }
private:

	UILAYER_ORDER				m_eUILayerOrder;
	vector<CUIObject*>			m_vecUIObject;

	bool						m_isActive;
public:
	static CUILayer* Create(UILAYER_ORDER eLayerOrder);
	virtual void		Free() override;
};

NS_END