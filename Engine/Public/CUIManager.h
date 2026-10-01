#pragma once
#include "CBase.h"

namespace Engine
{
	class CCanvas;
	class CBlendState;
	class CDepthStencilState;
	class CScreenFadeUI;
	class CGameManager;
}

NS_BEGIN(Engine)

class CUIManager : public CBase
{
private:
	explicit CUIManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CUIManager() {}
public:
	void					Pre_Update(float DT);
	void					Update(float DT);
	void					Late_Update(float DT);
	HRESULT					Render();
public:
	HRESULT					Ready_DefaultUI();
private:
	HRESULT					Initialize();
	HRESULT					Ready_DefaultCanvas();
	HRESULT					Ready_RenderState();
public:
	void					Fade_Out(float fTime);
	void					Fade_In(float fTime);
public:
	HRESULT					Add_Canvas(uint32 iIndex,CCanvas* pCanvas);
	CCanvas*				Get_Canvas(uint32 iIndex);
	void					Release_Canvas(uint32 iClearIndex);
private:
	/* Defualt Canvas */
	CScreenFadeUI*			m_pScreenFadeUI;
	CCanvas*				m_pDefaultCanvas;
	map<uint32,CCanvas*>	m_mapCanvas;
private:

	CBlendState*			m_pBlendState;		
	CDepthStencilState*		m_pDSV;
	CGameManager*			m_pGameManager;
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	static CUIManager* Create(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual void Free()override;
};

NS_END
