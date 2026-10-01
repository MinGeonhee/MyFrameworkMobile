#include "CTexture.h"
#include "CShader.h"
#include "CGraphicDevice.h"
#include "BCDecoder.h"

/* DirectXTK 의 WIC / DDSTextureLoader 대신 엔진 내부 로더를 사용한다 (Android 에도 같은 코드가 동작) */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

namespace
{
#pragma pack(push, 1)
	struct DDS_PIXELFORMAT
	{
		uint32 dwSize;
		uint32 dwFlags;
		uint32 dwFourCC;
		uint32 dwRGBBitCount;
		uint32 dwRBitMask;
		uint32 dwGBitMask;
		uint32 dwBBitMask;
		uint32 dwABitMask;
	};

	struct DDS_HEADER
	{
		uint32          dwSize;
		uint32          dwFlags;
		uint32          dwHeight;
		uint32          dwWidth;
		uint32          dwPitchOrLinearSize;
		uint32          dwDepth;
		uint32          dwMipMapCount;
		uint32          dwReserved1[11];
		DDS_PIXELFORMAT ddspf;
		uint32          dwCaps;
		uint32          dwCaps2;
		uint32          dwCaps3;
		uint32          dwCaps4;
		uint32          dwReserved2;
	};

	struct DDS_HEADER_DXT10
	{
		uint32 dxgiFormat;
		uint32 resourceDimension;
		uint32 miscFlag;
		uint32 arraySize;
		uint32 miscFlags2;
	};
#pragma pack(pop)

	constexpr uint32 MakeFourCC(char a, char b, char c, char d)
	{
		return static_cast<uint32>(static_cast<uint8>(a)) | (static_cast<uint32>(static_cast<uint8>(b)) << 8) |
			(static_cast<uint32>(static_cast<uint8>(c)) << 16) | (static_cast<uint32>(static_cast<uint8>(d)) << 24);
	}

	enum class PIXEL_KIND { RGBA8, BGRA8, BGRX8, R8, RG8, RGBA16F, RGBA32F, BC1, BC2, BC3, BC4, BC5, BC7, UNKNOWN };

	struct FORMAT_INFO
	{
		PIXEL_KIND				eKind{ PIXEL_KIND::UNKNOWN };
		bool					isSRGB{ false };
	};

	bool Is_Block(PIXEL_KIND eKind) { return eKind >= PIXEL_KIND::BC1 && eKind <= PIXEL_KIND::BC7; }

	uint32 Block_Bytes(PIXEL_KIND eKind)
	{
		return (eKind == PIXEL_KIND::BC1 || eKind == PIXEL_KIND::BC4) ? 8u : 16u;
	}

	uint32 Pixel_Bytes(PIXEL_KIND eKind)
	{
		switch (eKind)
		{
		case PIXEL_KIND::R8: return 1;
		case PIXEL_KIND::RG8: return 2;
		case PIXEL_KIND::RGBA16F: return 8;
		case PIXEL_KIND::RGBA32F: return 16;
		default: return 4;
		}
	}

	Diligent::TEXTURE_FORMAT To_Diligent(const FORMAT_INFO& tInfo)
	{
		switch (tInfo.eKind)
		{
		case PIXEL_KIND::RGBA8:   return tInfo.isSRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM;
		case PIXEL_KIND::BGRA8:   return tInfo.isSRGB ? Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB : Diligent::TEX_FORMAT_BGRA8_UNORM;
		case PIXEL_KIND::BGRX8:   return tInfo.isSRGB ? Diligent::TEX_FORMAT_BGRX8_UNORM_SRGB : Diligent::TEX_FORMAT_BGRX8_UNORM;
		case PIXEL_KIND::R8:      return Diligent::TEX_FORMAT_R8_UNORM;
		case PIXEL_KIND::RG8:     return Diligent::TEX_FORMAT_RG8_UNORM;
		case PIXEL_KIND::RGBA16F: return Diligent::TEX_FORMAT_RGBA16_FLOAT;
		case PIXEL_KIND::RGBA32F: return Diligent::TEX_FORMAT_RGBA32_FLOAT;
		case PIXEL_KIND::BC1:     return tInfo.isSRGB ? Diligent::TEX_FORMAT_BC1_UNORM_SRGB : Diligent::TEX_FORMAT_BC1_UNORM;
		case PIXEL_KIND::BC2:     return tInfo.isSRGB ? Diligent::TEX_FORMAT_BC2_UNORM_SRGB : Diligent::TEX_FORMAT_BC2_UNORM;
		case PIXEL_KIND::BC3:     return tInfo.isSRGB ? Diligent::TEX_FORMAT_BC3_UNORM_SRGB : Diligent::TEX_FORMAT_BC3_UNORM;
		case PIXEL_KIND::BC4:     return Diligent::TEX_FORMAT_BC4_UNORM;
		case PIXEL_KIND::BC5:     return Diligent::TEX_FORMAT_BC5_UNORM;
		case PIXEL_KIND::BC7:     return tInfo.isSRGB ? Diligent::TEX_FORMAT_BC7_UNORM_SRGB : Diligent::TEX_FORMAT_BC7_UNORM;
		default:                  return Diligent::TEX_FORMAT_UNKNOWN;
		}
	}

	FORMAT_INFO From_DXGI(uint32 iDXGI)
	{
		switch (iDXGI)
		{
		case 28: return { PIXEL_KIND::RGBA8, false };
		case 29: return { PIXEL_KIND::RGBA8, true };
		case 87: return { PIXEL_KIND::BGRA8, false };
		case 91: return { PIXEL_KIND::BGRA8, true };
		case 88: return { PIXEL_KIND::BGRX8, false };
		case 93: return { PIXEL_KIND::BGRX8, true };
		case 61: return { PIXEL_KIND::R8, false };
		case 49: return { PIXEL_KIND::RG8, false };
		case 10: return { PIXEL_KIND::RGBA16F, false };
		case 2:  return { PIXEL_KIND::RGBA32F, false };
		case 71: return { PIXEL_KIND::BC1, false };
		case 72: return { PIXEL_KIND::BC1, true };
		case 74: return { PIXEL_KIND::BC2, false };
		case 75: return { PIXEL_KIND::BC2, true };
		case 77: return { PIXEL_KIND::BC3, false };
		case 78: return { PIXEL_KIND::BC3, true };
		case 80: return { PIXEL_KIND::BC4, false };
		case 83: return { PIXEL_KIND::BC5, false };
		case 98: return { PIXEL_KIND::BC7, false };
		case 99: return { PIXEL_KIND::BC7, true };
		default: return {};
		}
	}

	FORMAT_INFO From_Legacy(const DDS_PIXELFORMAT& tPF)
	{
		constexpr uint32 DDPF_FOURCC = 0x4;
		constexpr uint32 DDPF_RGB = 0x40;
		constexpr uint32 DDPF_LUMINANCE = 0x20000;
		if (tPF.dwFlags & DDPF_FOURCC)
		{
			if (tPF.dwFourCC == MakeFourCC('D', 'X', 'T', '1')) return { PIXEL_KIND::BC1, false };
			if (tPF.dwFourCC == MakeFourCC('D', 'X', 'T', '2') || tPF.dwFourCC == MakeFourCC('D', 'X', 'T', '3')) return { PIXEL_KIND::BC2, false };
			if (tPF.dwFourCC == MakeFourCC('D', 'X', 'T', '4') || tPF.dwFourCC == MakeFourCC('D', 'X', 'T', '5')) return { PIXEL_KIND::BC3, false };
			if (tPF.dwFourCC == MakeFourCC('A', 'T', 'I', '1') || tPF.dwFourCC == MakeFourCC('B', 'C', '4', 'U')) return { PIXEL_KIND::BC4, false };
			if (tPF.dwFourCC == MakeFourCC('A', 'T', 'I', '2') || tPF.dwFourCC == MakeFourCC('B', 'C', '5', 'U')) return { PIXEL_KIND::BC5, false };
			if (tPF.dwFourCC == 113) return { PIXEL_KIND::RGBA16F, false };
			if (tPF.dwFourCC == 116) return { PIXEL_KIND::RGBA32F, false };
			return {};
		}
		if ((tPF.dwFlags & DDPF_RGB) && tPF.dwRGBBitCount == 32)
		{
			if (tPF.dwRBitMask == 0x000000ff && tPF.dwGBitMask == 0x0000ff00 && tPF.dwBBitMask == 0x00ff0000) return { PIXEL_KIND::RGBA8, false };
			if (tPF.dwRBitMask == 0x00ff0000 && tPF.dwGBitMask == 0x0000ff00 && tPF.dwBBitMask == 0x000000ff)
				return tPF.dwABitMask != 0 ? FORMAT_INFO{ PIXEL_KIND::BGRA8, false } : FORMAT_INFO{ PIXEL_KIND::BGRX8, false };
		}
		if ((tPF.dwFlags & DDPF_LUMINANCE) && tPF.dwRGBBitCount == 8) return { PIXEL_KIND::R8, false };
		return {};
	}

	HRESULT Load_DDS(Device* pDevice, const std::vector<uint8_t>& vecFile, const std::wstring& wstrPath, Diligent::ITexture** ppTexture)
	{
		if (vecFile.size() < 4 + sizeof(DDS_HEADER) || std::memcmp(vecFile.data(), "DDS ", 4) != 0)
			return E_FAIL;

		DDS_HEADER tHeader{};
		std::memcpy(&tHeader, vecFile.data() + 4, sizeof(DDS_HEADER));
		size_t iOffset = 4 + sizeof(DDS_HEADER);

		FORMAT_INFO tFormat{};
		uint32 iArraySize = 1;
		bool isCube = (tHeader.dwCaps2 & 0x200) != 0;

		if ((tHeader.ddspf.dwFlags & 0x4) && tHeader.ddspf.dwFourCC == MakeFourCC('D', 'X', '1', '0'))
		{
			if (vecFile.size() < iOffset + sizeof(DDS_HEADER_DXT10))
				return E_FAIL;
			DDS_HEADER_DXT10 tDX10{};
			std::memcpy(&tDX10, vecFile.data() + iOffset, sizeof(DDS_HEADER_DXT10));
			iOffset += sizeof(DDS_HEADER_DXT10);
			tFormat = From_DXGI(tDX10.dxgiFormat);
			iArraySize = (std::max)(1u, tDX10.arraySize);
			if (tDX10.miscFlag & 0x4)
				isCube = true;
		}
		else
			tFormat = From_Legacy(tHeader.ddspf);

		if (tFormat.eKind == PIXEL_KIND::UNKNOWN)
		{
			OutputDebugStringW((L"[Texture] Unsupported DDS format : " + wstrPath + L"\n").c_str());
			return E_FAIL;
		}

		const uint32 iWidth = tHeader.dwWidth;
		const uint32 iHeight = tHeader.dwHeight;
		const uint32 iMipLevels = (std::max)(1u, tHeader.dwMipMapCount);
		const uint32 iFaces = isCube ? 6u * iArraySize : iArraySize;

		CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
		const Diligent::TEXTURE_FORMAT eNativeFormat = To_Diligent(tFormat);
		const bool isNative = pGraphicDevice != nullptr && pGraphicDevice->Is_TextureFormatSupported(eNativeFormat);
		/* 장치가 BC 포맷을 지원하지 않으면 (대부분의 Android) CPU 에서 RGBA8 로 풀어서 올린다 */
		const bool isDecode = !isNative && Is_Block(tFormat.eKind);
		if (!isNative && !isDecode)
			return E_FAIL;

		std::vector<std::vector<uint8_t>> vecDecoded;
		std::vector<Diligent::TextureSubResData> vecSubRes;
		vecSubRes.reserve(iFaces * iMipLevels);
		if (isDecode)
			vecDecoded.reserve(iFaces * iMipLevels);

		for (uint32 iFace = 0; iFace < iFaces; ++iFace)
		{
			uint32 iMipWidth = iWidth;
			uint32 iMipHeight = iHeight;
			for (uint32 iMip = 0; iMip < iMipLevels; ++iMip)
			{
				size_t iRowPitch = 0, iSliceSize = 0;
				if (Is_Block(tFormat.eKind))
				{
					const uint32 iBlocksX = (std::max)(1u, (iMipWidth + 3) / 4);
					const uint32 iBlocksY = (std::max)(1u, (iMipHeight + 3) / 4);
					iRowPitch = static_cast<size_t>(iBlocksX) * Block_Bytes(tFormat.eKind);
					iSliceSize = iRowPitch * iBlocksY;
				}
				else
				{
					iRowPitch = static_cast<size_t>(iMipWidth) * Pixel_Bytes(tFormat.eKind);
					iSliceSize = iRowPitch * iMipHeight;
				}

				if (iOffset + iSliceSize > vecFile.size())
					return E_FAIL;

				const uint8_t* pSrc = vecFile.data() + iOffset;
				if (isDecode)
				{
					std::vector<uint8_t> vecRGBA;
					Engine::BC_Decompress(static_cast<int>(tFormat.eKind) - static_cast<int>(PIXEL_KIND::BC1) + 1, pSrc, iMipWidth, iMipHeight, vecRGBA);
					vecDecoded.push_back(std::move(vecRGBA));
					Diligent::TextureSubResData tSub{};
					tSub.pData = vecDecoded.back().data();
					tSub.Stride = static_cast<Diligent::Uint64>(iMipWidth) * 4;
					vecSubRes.push_back(tSub);
				}
				else
				{
					Diligent::TextureSubResData tSub{};
					tSub.pData = pSrc;
					tSub.Stride = iRowPitch;
					vecSubRes.push_back(tSub);
				}

				iOffset += iSliceSize;
				iMipWidth = (std::max)(1u, iMipWidth / 2);
				iMipHeight = (std::max)(1u, iMipHeight / 2);
			}
		}

		const std::string strName = WString_To_UTF8(wstrPath);
		Diligent::TextureDesc tDesc{};
		tDesc.Name = strName.c_str();
		tDesc.Type = isCube ? (iArraySize > 1 ? Diligent::RESOURCE_DIM_TEX_CUBE_ARRAY : Diligent::RESOURCE_DIM_TEX_CUBE)
			: (iArraySize > 1 ? Diligent::RESOURCE_DIM_TEX_2D_ARRAY : Diligent::RESOURCE_DIM_TEX_2D);
		tDesc.Width = iWidth;
		tDesc.Height = iHeight;
		tDesc.ArraySize = iFaces;
		tDesc.MipLevels = iMipLevels;
		tDesc.Format = isDecode ? (tFormat.isSRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM) : eNativeFormat;
		tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		tDesc.Usage = Diligent::USAGE_IMMUTABLE;

		Diligent::TextureData tData{ vecSubRes.data(), static_cast<Diligent::Uint32>(vecSubRes.size()) };
		pDevice->CreateTexture(tDesc, &tData, ppTexture);
		return *ppTexture != nullptr ? S_OK : E_FAIL;
	}
}

NS_BEGIN(Engine)

HRESULT Create_TextureRGBA8(Device* pDevice, uint32 iWidth, uint32 iHeight, const void* pPixels, const char* pName, Diligent::ITexture** ppTexture)
{
	if (pDevice == nullptr || pPixels == nullptr || ppTexture == nullptr || iWidth == 0 || iHeight == 0)
		return E_FAIL;

	Diligent::TextureDesc tDesc{};
	tDesc.Name = pName;
	tDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
	tDesc.Width = iWidth;
	tDesc.Height = iHeight;
	tDesc.MipLevels = 1;
	tDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
	tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	tDesc.Usage = Diligent::USAGE_IMMUTABLE;

	Diligent::TextureSubResData tSub{ pPixels, static_cast<Diligent::Uint64>(iWidth) * 4 };
	Diligent::TextureData tData{ &tSub, 1 };
	pDevice->CreateTexture(tDesc, &tData, ppTexture);
	return *ppTexture != nullptr ? S_OK : E_FAIL;
}

HRESULT Load_TextureFromFile(Device* pDevice, const wstring& wstrPath, Diligent::ITexture** ppTexture)
{
	if (pDevice == nullptr || ppTexture == nullptr)
		return E_FAIL;

	std::vector<uint8_t> vecFile;
	if (!File_ReadAll(wstrPath, vecFile) || vecFile.empty())
	{
		OutputDebugStringW((L"[Texture] File not found : " + wstrPath + L"\n").c_str());
		return E_FAIL;
	}

	std::wstring wstrEXT = std::filesystem::path(wstrPath).extension().wstring();
	std::transform(wstrEXT.begin(), wstrEXT.end(), wstrEXT.begin(), ::towlower);

	if (wstrEXT == L".dds")
		return Load_DDS(pDevice, vecFile, wstrPath, ppTexture);

	int iWidth = 0, iHeight = 0, iChannels = 0;
	stbi_uc* pPixels = stbi_load_from_memory(vecFile.data(), static_cast<int>(vecFile.size()), &iWidth, &iHeight, &iChannels, STBI_rgb_alpha);
	if (pPixels == nullptr)
	{
		OutputDebugStringW((L"[Texture] Decode failed : " + wstrPath + L"\n").c_str());
		return E_FAIL;
	}

	const std::string strName = WString_To_UTF8(wstrPath);
	const HRESULT hr = Create_TextureRGBA8(pDevice, static_cast<uint32>(iWidth), static_cast<uint32>(iHeight), pPixels, strName.c_str(), ppTexture);
	stbi_image_free(pPixels);
	return hr;
}

NS_END

CTexture::CTexture(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CResource(pDevice,pContext), m_wstrFileName{}, m_iTextureCnt{}
{
	m_eResourceType = RESOURCE_TYPE::TEXTURE;
}

CTexture::CTexture(const CTexture& Prototype)
	: CResource(Prototype), m_wstrFileName(Prototype.m_wstrFileName), m_vecTexture(Prototype.m_vecTexture), m_iTextureCnt(Prototype.m_iTextureCnt)
{
	for (auto& pTexture : m_vecTexture)
		Safe_AddRef(pTexture);
}

HRESULT CTexture::Initialize(const wstring& Path, uint32 iTextureCnt)
{
	/* 파일 경로 3분활 */
	std::filesystem::path FilePath(Path);
	std::wstring wstrEXT = FilePath.extension().wstring();
	std::transform(wstrEXT.begin(), wstrEXT.end(), wstrEXT.begin(), ::towlower);

	for (uint32 i = 0; i < iTextureCnt; ++i)
	{
		/* 원본 : wsprintf(szResult , Path , i) 로 "%d" 를 번호로 치환 */
		wchar_t szResult[MAX_PATH] = {};
		swprintf(szResult, MAX_PATH, Path.c_str(), i);
		wstring Result = szResult;

		Diligent::ITexture* pTexture = nullptr;
		if (FAILED(Load_TextureFromFile(m_pDevice.Get(), Result, &pTexture)) || pTexture == nullptr)
		{
			MessageBoxW(nullptr, (L"Texture Load Failed : " + Result).c_str(), L"System Message", MB_OK);
			return E_FAIL;
		}

		ShaderResourceView* pSRV = pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		NULLPTR_RETURN_EFAIL(pSRV);
		/* View 가 Texture 를 참조로 붙잡고 있으므로 View 만 보관한다 */
		pSRV->AddRef();
		pTexture->Release();

		m_vecTexture.push_back(pSRV);
	}

	m_wstrFileName = FilePath.stem().wstring();
	m_iTextureCnt = iTextureCnt;

	return S_OK;
}

HRESULT CTexture::Initialize(Texture2D* pTexture2DCom)
{
	if (pTexture2DCom == nullptr) return E_FAIL;

	ShaderResourceView* pSRV = pTexture2DCom->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	NULLPTR_RETURN_EFAIL(pSRV);
	pSRV->AddRef();

	m_vecTexture.push_back(pSRV);
	m_iTextureCnt = Cast_Uint(m_vecTexture.size());

	return S_OK;
}

HRESULT		CTexture::Copy_Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTexture::Set_GPU_Texture(CShader* pShader)
{
	NULLPTR_RETURN_EFAIL(pShader);

	return pShader->Set_GPU_Texture(m_vecTexture[0]);
}

HRESULT CTexture::Set_GPU_Textures(CShader* pShader, uint32 iCountTex, uint32 iOffset)
{
	NULLPTR_RETURN_EFAIL(pShader);

	return pShader->Set_GPU_Textures(iCountTex, &m_vecTexture[0], iOffset);
}

HRESULT CTexture::Set_GPU_MixTextureArr(CShader* pShader, uint32 iCountTex, uint32 iOffset)
{
	NULLPTR_RETURN_EFAIL(pShader);

	return pShader->Set_GPU_MixTextureArr(iCountTex, &m_vecTexture[0], iOffset);
}

HRESULT CTexture::Set_GPU_MaskTexture(CShader* pShader)
{
	NULLPTR_RETURN_EFAIL(pShader);

	return pShader->Set_GPU_MaskTexture(m_vecTexture[0]);
}

CTexture* CTexture::Create(const wstring& Path, uint32 iTextureCnt, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CTexture* pInstance = new CTexture(pDevice,pContext);

	if (FAILED(pInstance->Initialize(Path, iTextureCnt)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Texture is failed to Create");
		return nullptr;
	}

	return pInstance;
}

CTexture* CTexture::Create(Texture2D* pTexture2DCom,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CTexture* pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pTexture2DCom)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Texture is failed to Create");
		return nullptr;
	}

	return pInstance;
}

CResource* CTexture::Copy_Resource(void* pArg)
{
	CTexture* pCopyTexture = new CTexture(*this);

	if (FAILED(pCopyTexture->Copy_Initialize(pArg)))
	{
		Safe_Release(pCopyTexture);
		MSG_BOX("Copy Resource is failed To Texture");
		return nullptr;
	}

	return pCopyTexture;
}

void CTexture::Free()
{
	__super::Free();
	/* CTexture::Fee()*/

	for (auto& Texture : m_vecTexture)
		Safe_Release(Texture);
	m_vecTexture.clear();
}
