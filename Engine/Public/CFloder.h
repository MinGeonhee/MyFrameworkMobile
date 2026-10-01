#pragma once
#include "CBase.h"
/* 내가 지정한경로가 Root Floder가 된다 */

namespace Engine
{
	class CGameManager;
	class CFile;
}


NS_BEGIN(Engine)

class ENGINE_DLL CFloder :public CBase
{
	using Floder = CFloder;
	using FLODER = Floder;
private:
	explicit CFloder();
	virtual ~CFloder() {}
private:
	HRESULT							Initialize(CFloder* pParentFloder,CONST wstring& wstrRootPath,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);

public:
	bool							IsRootFloder()	CONST {return m_pParentFloder == nullptr ? true : false; }
	const map<wstring, FLODER*>&	GetFloders()	CONST{ return m_mapTreeFloder;}
	const vector<CFile*>&			GetFiles()		CONST{ return m_vecFile;}
	const FLODER_INFO&				GetFloderInfo() CONST{ return m_tInfo; }
private:

	vector<CFile*>					m_vecFile;
	map<wstring, FLODER*>			m_mapTreeFloder;
	FLODER_INFO						m_tInfo;
	CFloder*						m_pParentFloder;



#ifdef _DEBUG

	void							ImGui_Update();

#endif // _DEBUG
private:
	CGameManager*					m_pGameManager;
public:
	static	CFloder* Create(CFloder* pParentFloder,CONST wstring& wstrPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END