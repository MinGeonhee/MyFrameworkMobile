#pragma once
#include "CComponent.h"


namespace Engine
{
	class CCanvas;
}

NS_BEGIN(Engine)

class CUIComponent abstract : public CComponent
{
protected:
	typedef struct UIComponent_Desc
	{

	}UICOMPONENT_DESC;
protected:
	explicit		CUIComponent(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	explicit		CUIComponent(const CUIComponent& Proto);
	virtual			~CUIComponent();
protected:
	HRESULT			Proto_Initialize();
	virtual HRESULT Clone_Initialize(void* pArg);
public:
	virtual void    Pre_Update(float DT);
	virtual void    Update(float DT);
	virtual void    Late_Update(float DT);
	virtual HRESULT	Render(CCanvas* pCanvas);
public:
	virtual void Free()override;
};


NS_END