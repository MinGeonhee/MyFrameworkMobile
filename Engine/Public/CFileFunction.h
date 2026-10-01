#pragma once
#include "Engine_Define.h"

namespace Engine
{
	class CCollider;
}


NS_BEGIN(Engine)

class ENGINE_DLL CFileFunction
{
public:
	/*  */
	static  ShaderResourceView*			CreateShaderResourceViewReadFile(const std::wstring& _wstrFilePath ,ComPtr<Device> _pDeviceCom);
	static	vector<ShaderResourceView*>	CreateShaderResourceViewsReadFile( const std::wstring& _wstrFilePath, uint32 iCountTex,ComPtr<Device> _pDevice);


	/*  Save Collider Data By Json */
	/*  Save Collider Data By Json */ 
public:
	static HRESULT								Save_ColliderData(const std::wstring& _wstrModelName , const CCollider* pCollider);
	/* 이게 문제점이 여기서 Collider를 생성해버리는 불상사가 일어나면안되긴하는데 Clone때려버릴까? */
	static CCollider*							Load_ColliderData(const std::wstring& _wstrModelName);
};

NS_END

