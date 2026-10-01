#pragma once
#include "CGameObject.h"
#include "CUITransform.h"


namespace Engine
{
	class CGameManager;
	class CUIComponent;
	class CCavas;
}


NS_BEGIN(Engine)

using UI_Func = std::function<void()>;

class ENGINE_DLL CUIObject : public CBase
{
public:
	typedef struct UIObject_Desc : public CUITransform::UITRANSFORM_DESC
	{
		wstring		wstrName;
		UI_Func		funcEvent {nullptr};
		UI_DEPTH	eUIDepth  = UI_DEPTH::END;
	}UIOBJECT_DESC;
protected:
	explicit CUIObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CUIObject() = default;
protected:
	HRESULT					Initialize(void* pArg);
	virtual HRESULT			Ready_Component(void* pArg);
public:
	virtual void			Pre_Update(CCanvas* pCanvas,float DT);
	virtual void			Update(CCanvas* pCanvas,float DT);
	virtual void			Late_Update(CCanvas* pCanvas,float DT);
	virtual HRESULT			Render(CCanvas* pCanvas);
public:
	void					Set_UITransformMoveEvent(bool isMove);
	void					Set_IsActive(bool isActive);
	void					Set_UIDepth(UI_DEPTH eDepth) { m_eDepth = eDepth; }
	void					Set_Name(const wstring& Name) { m_wstrName = m_wstrName; }
public:
	bool					Get_UITarnsformMoveEvent();
	bool					Get_IsActive()		const { return m_isActive; }
	CUITransform*			Get_UITransform()	const { return m_pUITransformCom; }
	const wstring&			Get_Name()			const { return m_wstrName; }
	const UI_DEPTH&			Get_UIDepth()		const { return m_eDepth; }
public:
	HRESULT					Add_UIComponent(uint32 iFindSceneID, CONST std::wstring& wstrFindProtoName, CONST std::wstring& wstrAddName, void* pArg, FD CUIComponent** ppOut);
	CUIComponent*			Get_UIComponent(const wstring& wstrFindKey);
protected:
	wstring					m_wstrName;
	UI_DEPTH				m_eDepth;
	bool					m_isActive;
protected:
	UI_Func									m_funcEvent;
	unordered_map<wstring,CUIComponent*>	m_umapUIComponent;
	vector<CUIComponent*>					m_vecUIComponent;
	CUITransform*							m_pUITransformCom;
protected:
	CGameManager*							m_pGameManager;
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
public:
	static  CUIObject*		Create(void* pArg,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void			Free() override;
};

NS_END