#include "CMaterial.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CFileFunction.h"
#include "CTexture.h"

namespace
{
	/* "Name.%d.ext" 형식의 연속 텍스처 개수 (0 부터 끊기는 곳까지). Android assets 에서도 동작하도록 File_Exists 사용 */
	uint32 Count_SequenceFiles(const wstring& wstrFormatPath)
	{
		uint32 iCount = 0;
		wchar_t szPath[MAX_PATH] = {};
		for (;; ++iCount)
		{
			swprintf(szPath, MAX_PATH, wstrFormatPath.c_str(), iCount);
			if (!File_Exists(szPath))
				return iCount;
		}
	}
}



ShaderResourceView* CMaterial::m_pSRVs[Cast_Uint(MATERIAL_TYPE::END)] = {};

CMaterial::CMaterial(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CResource(pDevice,pContext), m_arrayMaterialTextures{}
	, m_pDefaultWhiteSRV(nullptr), m_iIndex{}, m_wstrName{}
{
	m_eResourceType = RESOURCE_TYPE::MATERIAL;
}

CMaterial::CMaterial(const CMaterial& Proto)
	: CResource(Proto),m_arrayMaterialTextures{Proto.m_arrayMaterialTextures}
	, m_pDefaultWhiteSRV(Proto.m_pDefaultWhiteSRV), m_wstrName{Proto.m_wstrName}
{
	Safe_AddRef(m_pGameManager);
	Safe_AddRef(m_pDefaultWhiteSRV);

	for (size_t i = 0; i < m_arrayMaterialTextures.size(); ++i)
		for ( auto& pSRV : m_arrayMaterialTextures[i])
			Safe_AddRef(pSRV);

	memcpy(m_iIndex,Proto.m_iIndex, sizeof(Cast_Uint(MATERIAL_TYPE::END)));
}

HRESULT CMaterial::Proto_Initialize(const MTL_PATH& MtlPath, const wstring& wstrName)
{
	/* 받은 Path 경로로 Texture 생성해주기 */

	/* Diffuse */
	m_wstrName = wstrName;

	if (FAILED(CMaterial::Ready_Diffuse(MtlPath.wstrDiffusePath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_Ambient(MtlPath.wstrAmbientPath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_Normal(MtlPath.wstrNormalPath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_Specular(MtlPath.wstrSpecularPath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_Emissive(MtlPath.wstrEmissivePath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_Metal(MtlPath.wstrMetalPath)))
		return E_FAIL;
	
	
	if (FAILED(CMaterial::Ready_DefaultWhiteColorTexture()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMaterial::Copy_Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMaterial::Ready_Diffuse(const wstring& wstrDiffusePath)
{
	/* 존재하지 않는다면 기본값 1 1 1 1 로 채워놓은 텍스처를 생성해줘야한다. */
	if (wstrDiffusePath.empty())
		return S_OK;

	if (wstrDiffusePath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::DIFFUSE)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrDiffusePath,m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrDiffusePath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase( wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::DIFFUSE)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount , m_pDevice);


	}

	return S_OK;
}

HRESULT CMaterial::Ready_Ambient(const wstring& wstrAmbientPath)
{
	if (wstrAmbientPath.empty())
		return S_OK;

	if (wstrAmbientPath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::AMBIENT)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrAmbientPath, m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrAmbientPath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase(wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::AMBIENT)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount, m_pDevice);
	}

	return S_OK;
}

HRESULT CMaterial::Ready_Normal(const wstring& wstrNormalPath)
{
	if (wstrNormalPath.empty())
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::NORMAL)].push_back
			(m_pGameManager->Make_ShaderResourceViewColor(255,128,128,255,m_pDevice,m_pDeviceContext));
		return S_OK;
	}


	if (wstrNormalPath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::NORMAL)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrNormalPath, m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrNormalPath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase(wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::NORMAL)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount, m_pDevice);
	}

	return S_OK;
}

HRESULT CMaterial::Ready_Specular(const wstring& wstrSpecularPath)
{

	if (wstrSpecularPath.empty())
		return S_OK;


	if (wstrSpecularPath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::SPECULAR)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrSpecularPath, m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrSpecularPath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase(wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::SPECULAR)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount, m_pDevice);
	}

	return S_OK;
}

HRESULT CMaterial::Ready_Emissive(const wstring& wstrEmissivePath)
{
	if (wstrEmissivePath.empty())
		return S_OK;

	if (wstrEmissivePath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::EMISSIVE)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrEmissivePath, m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrEmissivePath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase(wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::EMISSIVE)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount, m_pDevice);
	}


	return S_OK;
}

HRESULT CMaterial::Ready_Metal(const wstring& wstrMetalPath)
{

	if (wstrMetalPath.empty())
		return S_OK;


	if (wstrMetalPath.find(L".0") == wstring::npos)
	{
		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::METAL)].push_back(CFileFunction::CreateShaderResourceViewReadFile(wstrMetalPath, m_pDevice));
	}
	else
	{
		filesystem::path FullFilePath(wstrMetalPath);
		filesystem::path PathFileName(FullFilePath.filename().stem());
		wstring wstrFileName = PathFileName.wstring();
		wstring wstrExt = FullFilePath.extension().wstring();
		wstring wstrCheckFileName = wstrFileName.erase(wstrFileName.find(L".0"));
		wstring wstrChangeFilepath = FullFilePath.replace_filename(wstrCheckFileName + L".%d" + wstrExt).wstring();

		const uint32 iCount = Count_SequenceFiles(wstrChangeFilepath);

		m_arrayMaterialTextures[Cast_Uint(MATERIAL_TYPE::METAL)] = CFileFunction::CreateShaderResourceViewsReadFile(wstrChangeFilepath, iCount, m_pDevice);
	}
	return S_OK;
}

HRESULT CMaterial::Ready_DefaultWhiteColorTexture()
{
	bool isEmpty = 0;
	for (uint32 i = 0; i < Cast_Uint(MATERIAL_TYPE::END); ++i)
	{
		if (m_arrayMaterialTextures[i].empty())
			isEmpty = true;
	}

	if (isEmpty == true)
		m_pDefaultWhiteSRV = CGameManager::GetInstance()->Make_ShaderResourceViewColor(255,255,255,255,m_pDevice,m_pDeviceContext);


	return S_OK;
}

int32 CMaterial::Get_MateiralIndex(MATERIAL_TYPE eType)
{
	if (m_arrayMaterialTextures[Cast_Uint(eType)].empty()) return -1;
	return static_cast<int32>(m_iIndex[Cast_Uint(eType)]);
}

HRESULT CMaterial::Set_GPU_Material(CShader* pShader)
{
	NULLPTR_RETURN_EFAIL(pShader);

	
	for (auto& SRV : m_pSRVs)
		SRV = nullptr;

	for (uint32 i = 0; i < Cast_Uint(MATERIAL_TYPE::END); ++i)
	{
		!m_arrayMaterialTextures[i].empty() ? m_pSRVs[i] = m_arrayMaterialTextures[i].at(m_iIndex[i]) : m_pSRVs[i] = m_pDefaultWhiteSRV;
	}

	pShader->Set_GPU_MaterialTextures(&m_pSRVs[0]);

	return S_OK;
}

CResource* CMaterial::Copy_Resource(void* pArg)
{
	CMaterial* pMaterial = new CMaterial(*this);

	if (FAILED(pMaterial->Copy_Initialize(pArg)))
	{
		Safe_Release(pMaterial);
		MSG_BOX("Copy Reosurce is Failed to Material");
		return nullptr;
	}

	return pMaterial;
}

CMaterial* CMaterial::Create(const MTL_PATH& strMtlPath, const wstring& wstrName,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CMaterial* pMaterial = new CMaterial(pDevice, pContext);

	if (FAILED(pMaterial->Proto_Initialize(strMtlPath, wstrName)))
	{
		Safe_Release(pMaterial);
		MSG_BOX("Material Clone is failed To Create");
		return NULLPTR;
	}
	return pMaterial;
}

void CMaterial::Free()
{
	Safe_Release(m_pGameManager);
	for (auto& pSRVs : m_arrayMaterialTextures)
		for (auto& pSRV : pSRVs)
			Safe_Release(pSRV);

	Safe_Release(m_pDefaultWhiteSRV);
}
