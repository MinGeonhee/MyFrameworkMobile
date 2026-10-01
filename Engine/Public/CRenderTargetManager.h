#pragma once
#include "CBase.h"

namespace Engine
{
	class CRenderTarget;
}

NS_BEGIN(Engine)

class CRenderTargetManager final : public CBase
{
private:
	explicit CRenderTargetManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CRenderTargetManager() {}
public:
	HRESULT	Add_RenderTarget(const wstring& wstrAddKey , uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat);
	HRESULT Add_MRT( const wstring& wstrAddMRTKey , const wstring& wstrFindRenderTargetKey );

	ShaderResourceView*	Get_ShaderResourceView(const wstring& wstrFindRenderTargetKey);

	/* Begin End 함수 */
	HRESULT	Begin_MRT(const wstring& wstrFindMRT);
	/* A-plan : bind without clearing, then clear only a camera rect */
	HRESULT	Begin_MRT_NoClear(const wstring& wstrFindMRT);
	HRESULT	Clear_MRT_Rect(const wstring& wstrFindMRT, const RECT_DESC& tRect);
	HRESULT Bind_ShaderResource(const wstring& wstrFindRenderTargetKey , const string& strBindTexName ,CShader* pShader);
	HRESULT End_MRT();
#ifdef _DEBUG
	HRESULT	Ready_Debug( const wstring& wstrFindRenderTargetKey ,float fX , float fY , float fSizeX ,float fSizeY);
	HRESULT	Render_Debug( const wstring& wstrFindRenderTargtKey ,CShader* pShader , CVIBuffer_Rect* pBuffer);
#endif

public:
	CRenderTarget*			Find_RenderTarget(const wstring& wstrFindKey);
	list<CRenderTarget*>*	Find_MRT(const wstring& wstrFindKey);
private:
	HRESULT Initialize();
private:
	unordered_map<wstring,CRenderTarget*>		m_umapRenderTarget;
	unordered_map<wstring,list<CRenderTarget*>> m_umapMRT;
private:
	/* Bind 하기 이전의 RTV , DSV 를 기록해둔다 */
	RenderTargetView*						m_pBackBuffer;
	DepthStencilView*						m_pPreDSV;
private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	static CRenderTargetManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END