#pragma once
#include "CBase.h"

namespace Engine
{
	class CTexture;
	class CModel;
}


NS_BEGIN(Engine)

class ENGINE_DLL CFile :public CBase
{
public:
	enum class FILE_TYPE
	{
	   NONE , TEXTURE , MODEL , TXT , JSON , BIN , END 
	};
	using CFILE = CFile;

public:
	explicit CFile();
	virtual ~CFile();
private:
	HRESULT	 Initialize(const wstring& wstrPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
public:
	CONST FILE_INFO&	Get_FileInfo()	CONST{ return m_tInfo; }
	CTexture*			Get_Image()		CONST{return m_pTexture == nullptr ? nullptr : m_pTexture; }
	CModel*				Get_Model()		CONST{return m_pModel == nullptr ? nullptr : m_pModel; }
	FILE_TYPE			Get_FileType()	CONST{ return m_eFileType; }
private:
	/* 파일 패스 알아서 조립해서 써야할수도? */
	FILE_INFO	 m_tInfo;
	
	FILE_TYPE	 m_eFileType;

	CModel*					m_pModel;
	CTexture*				m_pTexture;
public:
	static  CFile* Create(const wstring& wstrPath, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override; 
};

NS_END

