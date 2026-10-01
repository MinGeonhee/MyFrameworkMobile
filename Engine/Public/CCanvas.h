#pragma once

#include "CBase.h"

namespace Engine
{
	class CUILayer;
	class CUITransform;
	class CShader;
}

NS_BEGIN(Engine)

class ENGINE_DLL CCanvas final : public CBase
{
public:
	typedef struct Canvas_Desc final
	{
		wstring wstrShaderKey{};
		/* 도화지 전체 크기를 지정해준다 */
		Float2	vCanvasSize		{ 1.f,1.f };
		Float2	vCanvasStartPos { 0.f,0.f };
	}Canvas_DESC;

public:
	explicit CCanvas(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CCanvas() {}
private:
	HRESULT			Initialize(const Canvas_Desc* pDesc);
	HRESULT			Ready_Component(void* pDesc);
public:
	void			Pre_Update(float DT);
	void			Update(float DT);
	void			Late_Update(float DT);
	HRESULT			Render();
public:

	HRESULT			Add_UILayer(const wstring& wstrAddKey , CUILayer* pUILayer);
	CUILayer*		Get_UILayer(const wstring& wstrFindKey)	const;

public:
	void									 Set_StartX(float fStartX) { m_tCanvasViewPort.TopLeftX = fStartX; m_isChangeViewPort = true; }
	void									 Set_StartY(float fStartY) { m_tCanvasViewPort.TopLeftY = fStartY; m_isChangeViewPort = true;}
	void									 Set_SizeX(float fSizeX)   { m_tCanvasViewPort.Width = fSizeX; m_isChangeViewPort = true;}
	void									 Set_SizeY(float fSizeY)   { m_tCanvasViewPort.Height = fSizeY; m_isChangeViewPort = true;}
	void									 Set_StartPos(const Float2& vStartPos)	{ m_tCanvasViewPort.TopLeftX = vStartPos.x;m_tCanvasViewPort.TopLeftY = vStartPos.y; m_isChangeViewPort = true;	}
	void									 Set_Size(const Float2& vSize)			{ m_tCanvasViewPort.Width = vSize.x; m_tCanvasViewPort.Height = vSize.y; m_isChangeViewPort = true;}
	void									 Set_IsActive(bool isActive) { m_isActive = isActive; }
public:
	CShader*								 Get_Shader()		  const	{ return m_pShader; }
	bool									 Get_IsActive()		  const	{ return m_isActive; }
	const unordered_map<wstring, CUILayer*>& Get_UILayers()		  const	{ return m_umapUILayer; }
	const VIEWPORT_DESC&					 Get_CanvasViewPort() const { return m_tCanvasViewPort; }
	const Float4x4*							 Get_CanvasMatrixPtr()   const { return &m_matCanvas; }
private:
	/* UiLayer를 담을 컨테이너 필요한데...그냥 Key값으로 찾는게 편하겠지 Umap vs Map인데.. */
	unordered_map<wstring,CUILayer*>		m_umapUILayer;
private:
	bool									m_isActive;
	VIEWPORT_DESC							m_tCanvasViewPort;
	bool									m_isChangeViewPort;
	Float4x4								m_matCanvas;
	CShader*								m_pShader;
private:
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
public:
	static	CCanvas*		Create(const Canvas_Desc* pDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void			Free() override;
};

NS_END
