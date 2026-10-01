#pragma once
#include "CBase.h"


#ifdef _DEBUG

NS_BEGIN(Engine)


enum class ImGui_Style
{
	DARK,
	LIGHT,
};


class CImGuiWindow;

class ENGINE_DLL CImGuiManager : public CBase
{
	DECLARE_SINGLETON(CImGuiManager)

private:
	explicit CImGuiManager();
	virtual	~CImGuiManager();

public:

	CImGuiWindow*		Find_ImGuiWindow(uint32 ID);
	HRESULT				Add_ImGuiWindow(uint32 ID, CImGuiWindow* pImGuiWindow );

public:
	HRESULT				Initialize_ImGui(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext, ImGui_Style eStyle);
	void				Clear_ImGuiFrame();
	void				Update(float DT);
	HRESULT				Render();
private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
private:
	map< uint32 , CImGuiWindow*>	m_umapImGuiWindow;
public:
	virtual	void			Free() override;
};


NS_END


#endif // _DEBUG
