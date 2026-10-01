#include "CFileFunction.h"
#include "CGameManager.h"
#include "CJsonFunction.h"
#include "CFilePath.h"
#include "EnumUtils.h"
#include "CCollider.h"
#include "CTexture.h"

/* 단일 텍스처 로딩 */
ShaderResourceView* CFileFunction::CreateShaderResourceViewReadFile(const std::wstring& _wstrFilePath, ComPtr<Device> _pDeviceCom)
{
	if (_wstrFilePath.empty()) return nullptr;
	if (_pDeviceCom == nullptr)  return nullptr;

	std::wstring wstrFilePath = _wstrFilePath;

	filesystem::path fsPath(wstrFilePath);
	/* 파일 확장자 검사 */
	std::wstring fsPathEXT = fsPath.extension();

	/* 원본 : DDSTextureLoader / WICTextureLoader → 엔진 텍스처 로더 (png , jpg , tga , bmp , dds) */
	Diligent::ITexture* pResource{ nullptr };
	if (FAILED(Load_TextureFromFile(_pDeviceCom.Get(), wstrFilePath, &pResource)) || pResource == nullptr)
		return nullptr;

	ShaderResourceView* pSRV = pResource->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	if (pSRV != nullptr)
		pSRV->AddRef();
	Safe_Release(pResource);

	return pSRV;
}

vector<ShaderResourceView*> CFileFunction::CreateShaderResourceViewsReadFile(const std::wstring& _wstrFilePath, uint32 iCountTex  ,ComPtr<Device> _pDevice)
{
	if (_wstrFilePath.empty()) return	vector<ShaderResourceView*>();
	vector<ShaderResourceView*> vecSRV{};
	wstring wstrFilePath= _wstrFilePath;
	filesystem::path fsFullPath(wstrFilePath);
	filesystem::path fsName(fsFullPath.filename());
	filesystem::path fsExt(fsName.extension());
	/* 이거 실제로도 제거해버리니 조심하자... 반환해주는게아니라 실제 이 객체에서 지워버린다 */
	filesystem::path fsPath(fsFullPath.remove_filename());

	HRESULT hr{ E_FAIL };

	for (uint32 i = 0; i < iCountTex; ++i)
	{
		wchar_t szPath[MAX_PATH] = L"";

		/* i 채워주면될듯? */
		swprintf(szPath, MAX_PATH, fsName.wstring().c_str(), i);
		wstring FinalPath = fsPath.wstring() + szPath;

		ShaderResourceView* pSRV = CreateShaderResourceViewReadFile(FinalPath, _pDevice);
		hr = pSRV != nullptr ? S_OK : E_FAIL;


		if (hr == E_FAIL)
		{
			for (auto& SRV : vecSRV)
				Safe_Release(SRV);
			return vector<ShaderResourceView*>();
		}


		vecSRV.push_back(pSRV);
	}

	/* 파일명이 %d 을 찾아서 변환해주자...*/	
	return vecSRV;
}

HRESULT CFileFunction::Save_ColliderData(const std::wstring& _wstrModelName,const CCollider* pCollider)
{
	if (_wstrModelName.empty()) return E_FAIL;

	nlohmann::ordered_json json;

	CFileStream is(CFilePath::ColliderData_Path_Json);
	if (is.is_open())
	{
		is >> json;
		is.close();
	}

	string strName = WString_To_String(_wstrModelName);
	auto& Collider_Data = json[strName];
	COLLIDER_TYPE eType = pCollider->Get_ColliderType();


	ofstream os(CFilePath::ColliderData_Path_Json);
	os << json;
	os.close();
	
	return S_OK;
}

CCollider* CFileFunction::Load_ColliderData(const std::wstring& _wstrModelName)
{ 
	//if (_wstrModelName.empty()) return nullptr;
	//
	//CFileStream fs(CFilePath::ColliderData_Path_Json);

	//if (!fs.is_open()) return nullptr;

	//nlohmann::ordered_json json;

	//fs >> json;

	//fs.close();

	//string strName = WString_To_String(_wstrModelName);

	//Component_Desc tCom_Desc{};
	//COLLIDERCOM_DESC tCollider_Desc{};
	//tCom_Desc.pColliderCom_Desc = &tCollider_Desc;

	//CCollider* pCollider{ nullptr };

	//if (!json.contains(strName))
	//{
	//	pCollider = static_cast<CCollider*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, Proto_Com_ColliderBox, &tCom_Desc));
	//	return pCollider;
	//}

	//auto& Collider_Data = json[strName];

	//COLLIDER_TYPE eColliderType{};

	//CJsonFunction::Load_ColliderData(Collider_Data, eColliderType, tCollider_Desc.vColliderOffset , tCollider_Desc.vColliderScale);

	//if (eColliderType == COLLIDER_TYPE::SPHERE)
	//	pCollider = static_cast<CCollider*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT,Proto_Com_ColliderSphere, &tCom_Desc));
	//else if (eColliderType == COLLIDER_TYPE::BOX)
	//	pCollider = static_cast<CCollider*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, Proto_Com_ColliderBox, &tCom_Desc));
	//else if (eColliderType == COLLIDER_TYPE::ROTBOX)
	//	pCollider = static_cast<CCollider*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, Proto_Com_ColliderRotBox, &tCom_Desc));

	//NULLPTR_RETURN_NULLPTR(pCollider);

	return nullptr;
}




