#include "DirectXTK_Compat.h"
#include "CGraphicDevice.h"
#include "CTexture.h"
#include "BCDecoder.h"

#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"

#include <cwctype>

/* ============================================================================================== */
/*                         SpriteBatch / SpriteFont (DirectXTK 대체 구현)                            */
/* ============================================================================================== */

namespace
{
	constexpr const char* g_pSpriteShader = R"(
cbuffer SpriteConstants
{
    float4 g_vViewport;   // 1/Width , 1/Height , TopLeftX , TopLeftY
};

Texture2D    g_Texture;
SamplerState g_Texture_sampler;

struct VS_IN
{
    float2 vPosition : ATTRIB0;
    float2 vUV       : ATTRIB1;
    float4 vColor    : ATTRIB2;
};

struct PS_IN
{
    float4 vPosition : SV_Position;
    float2 vUV       : TEXCOORD0;
    float4 vColor    : COLOR0;
};

PS_IN VSMain(VS_IN In)
{
    PS_IN Out;
    float2 vNDC = (In.vPosition - g_vViewport.zw) * g_vViewport.xy * float2(2.f, -2.f) + float2(-1.f, 1.f);
    Out.vPosition = float4(vNDC, 0.f, 1.f);
    Out.vUV = In.vUV;
    Out.vColor = In.vColor;
    return Out;
}

float4 PSMain(PS_IN In) : SV_Target
{
    return g_Texture.Sample(g_Texture_sampler, In.vUV) * In.vColor;
}
)";

	struct SPRITE_VERTEX
	{
		XMFLOAT2 vPosition;
		XMFLOAT2 vUV;
		XMFLOAT4 vColor;
	};

	struct SPRITE_PSO
	{
		uint32										iNumRT = 0;
		Diligent::TEXTURE_FORMAT					eRTFormat = Diligent::TEX_FORMAT_UNKNOWN;
		Diligent::TEXTURE_FORMAT					eDSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
		Engine::ComPtr<Diligent::IPipelineState>	pPSO;
		Engine::ComPtr<Diligent::IShaderResourceBinding> pSRB;
	};

	struct SPRITE_RESOURCES
	{
		Engine::ComPtr<Diligent::IShader>	pVS;
		Engine::ComPtr<Diligent::IShader>	pPS;
		Engine::ComPtr<Diligent::IBuffer>	pVB;
		Engine::ComPtr<Diligent::IBuffer>	pCB;
		uint32								iVBCapacity = 0;
		std::vector<SPRITE_PSO>				vecPSO;
	};
	SPRITE_RESOURCES g_tSprite;

	void Upload(Engine::DeviceContext* pContext, Diligent::IBuffer* pBuffer, const void* pData, size_t iSize)
	{
		void* pMapped = nullptr;
		pContext->MapBuffer(pBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pMapped);
		if (pMapped != nullptr)
		{
			::memcpy(pMapped, pData, iSize);
			pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);
		}
	}

	SPRITE_PSO* Find_SpritePSO(Engine::Device* pDevice, const Engine::GRAPHIC_PIPELINE_STATE& tState)
	{
		const Diligent::TEXTURE_FORMAT eRT = tState.iNumRenderTargets > 0 ? tState.eRTVFormats[0] : Diligent::TEX_FORMAT_UNKNOWN;
		for (SPRITE_PSO& tPSO : g_tSprite.vecPSO)
			if (tPSO.iNumRT == tState.iNumRenderTargets && tPSO.eRTFormat == eRT && tPSO.eDSVFormat == tState.eDSVFormat)
				return &tPSO;

		if (g_tSprite.pVS == nullptr)
		{
			Diligent::ShaderCreateInfo tCI{};
			tCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
			tCI.Source = g_pSpriteShader;
			tCI.Desc.UseCombinedTextureSamplers = true;
			tCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			tCI.Desc.Name = "SpriteBatch VS";
			tCI.EntryPoint = "VSMain";
			pDevice->CreateShader(tCI, g_tSprite.pVS.ReleaseAndGetAddressOf());
			tCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			tCI.Desc.Name = "SpriteBatch PS";
			tCI.EntryPoint = "PSMain";
			pDevice->CreateShader(tCI, g_tSprite.pPS.ReleaseAndGetAddressOf());

			Diligent::BufferDesc tCBDesc{};
			tCBDesc.Name = "SpriteBatch CB";
			tCBDesc.Size = sizeof(XMFLOAT4);
			tCBDesc.Usage = Diligent::USAGE_DYNAMIC;
			tCBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			tCBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			pDevice->CreateBuffer(tCBDesc, nullptr, g_tSprite.pCB.ReleaseAndGetAddressOf());
			if (g_tSprite.pVS == nullptr || g_tSprite.pPS == nullptr || g_tSprite.pCB == nullptr)
				return nullptr;
		}

		Diligent::GraphicsPipelineStateCreateInfo tCI{};
		tCI.PSODesc.Name = "SpriteBatch PSO";
		tCI.GraphicsPipeline.NumRenderTargets = static_cast<Diligent::Uint8>(tState.iNumRenderTargets);
		tCI.GraphicsPipeline.RTVFormats[0] = eRT;
		tCI.GraphicsPipeline.DSVFormat = tState.eDSVFormat;
		tCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		tCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		tCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

		/* DirectXTK SpriteBatch 기본 : 미리 곱한 알파 (One , InvSrcAlpha) */
		Diligent::RenderTargetBlendDesc& tBlend = tCI.GraphicsPipeline.BlendDesc.RenderTargets[0];
		tBlend.BlendEnable = true;
		tBlend.SrcBlend = Diligent::BLEND_FACTOR_ONE;
		tBlend.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		tBlend.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
		tBlend.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;

		const Diligent::LayoutElement arrLayout[] = {
			Diligent::LayoutElement{ 0, 0, 2, Diligent::VT_FLOAT32, false },
			Diligent::LayoutElement{ 1, 0, 2, Diligent::VT_FLOAT32, false },
			Diligent::LayoutElement{ 2, 0, 4, Diligent::VT_FLOAT32, false },
		};
		tCI.GraphicsPipeline.InputLayout.LayoutElements = arrLayout;
		tCI.GraphicsPipeline.InputLayout.NumElements = 3;
		tCI.pVS = g_tSprite.pVS.Get();
		tCI.pPS = g_tSprite.pPS.Get();

		Diligent::SamplerDesc tSampler{};
		tSampler.MinFilter = tSampler.MagFilter = tSampler.MipFilter = Diligent::FILTER_TYPE_LINEAR;
		tSampler.AddressU = tSampler.AddressV = tSampler.AddressW = Diligent::TEXTURE_ADDRESS_CLAMP;
		Diligent::ImmutableSamplerDesc tImmutable{ Diligent::SHADER_TYPE_PIXEL, "g_Texture", tSampler };
		Diligent::ShaderResourceVariableDesc tVariable{ Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC };
		tCI.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		tCI.PSODesc.ResourceLayout.Variables = &tVariable;
		tCI.PSODesc.ResourceLayout.NumVariables = 1;
		tCI.PSODesc.ResourceLayout.ImmutableSamplers = &tImmutable;
		tCI.PSODesc.ResourceLayout.NumImmutableSamplers = 1;

		SPRITE_PSO tPSO{};
		tPSO.iNumRT = tState.iNumRenderTargets;
		tPSO.eRTFormat = eRT;
		tPSO.eDSVFormat = tState.eDSVFormat;
		pDevice->CreateGraphicsPipelineState(tCI, tPSO.pPSO.ReleaseAndGetAddressOf());
		if (tPSO.pPSO == nullptr)
			return nullptr;
		if (auto* pVar = tPSO.pPSO->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "SpriteConstants"))
			pVar->Set(g_tSprite.pCB.Get());
		tPSO.pPSO->CreateShaderResourceBinding(tPSO.pSRB.ReleaseAndGetAddressOf(), true);

		g_tSprite.vecPSO.push_back(tPSO);
		return &g_tSprite.vecPSO.back();
	}
}

namespace DirectX
{
	void SpriteBatch_ReleaseResources()
	{
		g_tSprite = SPRITE_RESOURCES{};
	}

	SpriteBatch::SpriteBatch(Engine::DeviceContext* pContext)
	{
	}

	SpriteBatch::~SpriteBatch()
	{
	}

	void SpriteBatch::Begin()
	{
		m_vecSprite.clear();
		m_isBegin = true;
	}

	void XM_CALLCONV SpriteBatch::Draw(Engine::ShaderResourceView* pTexture, const XMFLOAT2& vPosition, const RECT* pSourceRect, FXMVECTOR vColor,
		float fRotation, const XMFLOAT2& vOrigin, const XMFLOAT2& vScale, SpriteEffects eEffects, float fLayerDepth)
	{
		if (pTexture == nullptr)
			return;

		const Diligent::TextureDesc& tDesc = pTexture->GetTexture()->GetDesc();
		const float fTexW = static_cast<float>(tDesc.Width);
		const float fTexH = static_cast<float>(tDesc.Height);

		float fLeft = 0.f, fTop = 0.f, fRight = fTexW, fBottom = fTexH;
		if (pSourceRect != nullptr)
		{
			fLeft = static_cast<float>(pSourceRect->left);
			fTop = static_cast<float>(pSourceRect->top);
			fRight = static_cast<float>(pSourceRect->right);
			fBottom = static_cast<float>(pSourceRect->bottom);
		}

		const float fWidth = (fRight - fLeft) * vScale.x;
		const float fHeight = (fBottom - fTop) * vScale.y;
		const float fCos = cosf(fRotation), fSin = sinf(fRotation);

		SPRITE tSprite{};
		tSprite.pTexture = pTexture;
		XMStoreFloat4(&tSprite.vColor, vColor);

		const XMFLOAT2 arrLocal[4] = {
			{ -vOrigin.x * vScale.x,          -vOrigin.y * vScale.y },
			{ -vOrigin.x * vScale.x + fWidth, -vOrigin.y * vScale.y },
			{ -vOrigin.x * vScale.x + fWidth, -vOrigin.y * vScale.y + fHeight },
			{ -vOrigin.x * vScale.x,          -vOrigin.y * vScale.y + fHeight },
		};
		for (int i = 0; i < 4; ++i)
		{
			tSprite.vCorner[i].x = vPosition.x + arrLocal[i].x * fCos - arrLocal[i].y * fSin;
			tSprite.vCorner[i].y = vPosition.y + arrLocal[i].x * fSin + arrLocal[i].y * fCos;
		}

		float u0 = fLeft / fTexW, u1 = fRight / fTexW, v0 = fTop / fTexH, v1 = fBottom / fTexH;
		if (eEffects & SpriteEffects_FlipHorizontally) std::swap(u0, u1);
		if (eEffects & SpriteEffects_FlipVertically) std::swap(v0, v1);
		tSprite.vUV[0] = { u0, v0 };
		tSprite.vUV[1] = { u1, v0 };
		tSprite.vUV[2] = { u1, v1 };
		tSprite.vUV[3] = { u0, v1 };

		m_vecSprite.push_back(tSprite);
	}

	void SpriteBatch::End()
	{
		m_isBegin = false;
		if (m_vecSprite.empty())
			return;

		Engine::CGraphicDevice* pGraphicDevice = Engine::CGraphicDevice::Get_Instance();
		if (pGraphicDevice == nullptr)
			return;

		Engine::Device* pDevice = pGraphicDevice->Get_Device().Get();
		Engine::DeviceContext* pContext = pGraphicDevice->Get_DeviceContext().Get();

		SPRITE_PSO* pPSO = Find_SpritePSO(pDevice, pGraphicDevice->Get_PipelineState());
		if (pPSO == nullptr)
			return;

		const uint32 iVBSize = static_cast<uint32>(m_vecSprite.size() * 6 * sizeof(SPRITE_VERTEX));
		if (g_tSprite.pVB == nullptr || g_tSprite.iVBCapacity < iVBSize)
		{
			g_tSprite.iVBCapacity = (std::max)(iVBSize, g_tSprite.iVBCapacity * 2u);
			Diligent::BufferDesc tDesc{};
			tDesc.Name = "SpriteBatch VB";
			tDesc.Size = g_tSprite.iVBCapacity;
			tDesc.Usage = Diligent::USAGE_DYNAMIC;
			tDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			pDevice->CreateBuffer(tDesc, nullptr, g_tSprite.pVB.ReleaseAndGetAddressOf());
			if (g_tSprite.pVB == nullptr)
				return;
		}

		std::vector<SPRITE_VERTEX> vecVertex;
		vecVertex.reserve(m_vecSprite.size() * 6);
		static const int s_arrIndex[6] = { 0, 1, 2, 0, 2, 3 };
		for (const SPRITE& tSprite : m_vecSprite)
			for (int i : s_arrIndex)
				vecVertex.push_back(SPRITE_VERTEX{ tSprite.vCorner[i], tSprite.vUV[i], tSprite.vColor });
		Upload(pContext, g_tSprite.pVB.Get(), vecVertex.data(), vecVertex.size() * sizeof(SPRITE_VERTEX));

		uint32 iNumViewport = 0;
		Engine::VIEWPORT_DESC tViewport{};
		pGraphicDevice->RS_GetViewports(&iNumViewport, &tViewport);
		if (tViewport.Width <= 0.f || tViewport.Height <= 0.f)
			tViewport = pGraphicDevice->Get_OriginViewPort();
		const XMFLOAT4 vViewport{ 1.f / tViewport.Width, 1.f / tViewport.Height, tViewport.TopLeftX, tViewport.TopLeftY };
		Upload(pContext, g_tSprite.pCB.Get(), &vViewport, sizeof(vViewport));

		Diligent::IBuffer* pVB = g_tSprite.pVB.Get();
		const Diligent::Uint64 iOffset = 0;
		pContext->SetVertexBuffers(0, 1, &pVB, &iOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
		pContext->SetPipelineState(pPSO->pPSO.Get());

		Diligent::IShaderResourceVariable* pTextureVar = pPSO->pSRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture");

		/* 같은 텍스처가 연속되는 구간을 한 번에 그린다 */
		size_t iStart = 0;
		while (iStart < m_vecSprite.size())
		{
			size_t iEnd = iStart + 1;
			while (iEnd < m_vecSprite.size() && m_vecSprite[iEnd].pTexture == m_vecSprite[iStart].pTexture)
				++iEnd;

			if (pTextureVar != nullptr)
				pTextureVar->Set(m_vecSprite[iStart].pTexture);
			pContext->CommitShaderResources(pPSO->pSRB.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			Diligent::DrawAttribs tDraw{};
			tDraw.NumVertices = static_cast<Diligent::Uint32>((iEnd - iStart) * 6);
			tDraw.StartVertexLocation = static_cast<Diligent::Uint32>(iStart * 6);
			tDraw.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
			pContext->Draw(tDraw);

			iStart = iEnd;
		}

		m_vecSprite.clear();
		pGraphicDevice->Invalidate_BoundState();
	}

	SpriteFont::SpriteFont(Engine::Device* pDevice, const wchar_t* pFileName)
	{
		Engine::CFileStream tFile(std::wstring(pFileName != nullptr ? pFileName : L""));
		if (!tFile.is_open())
		{
			OutputDebugStringW((std::wstring(L"[SpriteFont] File not found : ") + (pFileName ? pFileName : L"") + L"\n").c_str());
			return;
		}

		char szMagic[8]{};
		tFile.read(szMagic, 8);
		if (std::memcmp(szMagic, "DXTKfont", 8) != 0)
			return;

		uint32_t iGlyphCount = 0;
		tFile.read(&iGlyphCount, sizeof(uint32_t));
		m_vecGlyph.resize(iGlyphCount);
		tFile.read(m_vecGlyph.data(), sizeof(Glyph) * iGlyphCount);

		tFile.read(&m_fLineSpacing, sizeof(float));
		tFile.read(&m_iDefaultCharacter, sizeof(uint32_t));

		uint32_t iWidth = 0, iHeight = 0, iFormat = 0, iStride = 0, iRows = 0;
		tFile.read(&iWidth, 4);
		tFile.read(&iHeight, 4);
		tFile.read(&iFormat, 4);
		tFile.read(&iStride, 4);
		tFile.read(&iRows, 4);

		std::vector<uint8_t> vecData(static_cast<size_t>(iStride) * iRows);
		tFile.read(vecData.data(), vecData.size());
		if (tFile.fail())
			return;

		/* 텍스처 : BC 압축은 CPU 에서 풀어서 RGBA8 로 만든다 (모든 장치에서 동일하게 동작) */
		std::vector<uint8_t> vecRGBA;
		switch (iFormat)
		{
		case 71: case 72: Engine::BC_Decompress(1, vecData.data(), iWidth, iHeight, vecRGBA); break;
		case 74: case 75: Engine::BC_Decompress(2, vecData.data(), iWidth, iHeight, vecRGBA); break;
		case 77: case 78: Engine::BC_Decompress(3, vecData.data(), iWidth, iHeight, vecRGBA); break;
		case 28: case 29:
			vecRGBA.resize(static_cast<size_t>(iWidth) * iHeight * 4);
			for (uint32_t y = 0; y < iHeight; ++y)
				std::memcpy(&vecRGBA[static_cast<size_t>(y) * iWidth * 4], &vecData[static_cast<size_t>(y) * iStride], static_cast<size_t>(iWidth) * 4);
			break;
		case 87:
			vecRGBA.resize(static_cast<size_t>(iWidth) * iHeight * 4);
			for (uint32_t y = 0; y < iHeight; ++y)
				for (uint32_t x = 0; x < iWidth; ++x)
				{
					const uint8_t* pSrc = &vecData[static_cast<size_t>(y) * iStride + x * 4];
					uint8_t* pDst = &vecRGBA[(static_cast<size_t>(y) * iWidth + x) * 4];
					pDst[0] = pSrc[2]; pDst[1] = pSrc[1]; pDst[2] = pSrc[0]; pDst[3] = pSrc[3];
				}
			break;
		case 115: /* B4G4R4A4 */
			vecRGBA.resize(static_cast<size_t>(iWidth) * iHeight * 4);
			for (uint32_t y = 0; y < iHeight; ++y)
				for (uint32_t x = 0; x < iWidth; ++x)
				{
					uint16_t c = 0;
					std::memcpy(&c, &vecData[static_cast<size_t>(y) * iStride + x * 2], 2);
					uint8_t* pDst = &vecRGBA[(static_cast<size_t>(y) * iWidth + x) * 4];
					pDst[2] = static_cast<uint8_t>((c & 0xF) * 17);
					pDst[1] = static_cast<uint8_t>(((c >> 4) & 0xF) * 17);
					pDst[0] = static_cast<uint8_t>(((c >> 8) & 0xF) * 17);
					pDst[3] = static_cast<uint8_t>(((c >> 12) & 0xF) * 17);
				}
			break;
		default:
			OutputDebugStringW(L"[SpriteFont] Unsupported texture format\n");
			return;
		}

		Diligent::ITexture* pTexture = nullptr;
		if (SUCCEEDED(Engine::Create_TextureRGBA8(pDevice, iWidth, iHeight, vecRGBA.data(), "SpriteFont", &pTexture)))
			m_pTexture.Attach(pTexture);
	}

	SpriteFont::~SpriteFont()
	{
	}

	const SpriteFont::Glyph* SpriteFont::Find_Glyph(wchar_t wcCharacter) const
	{
		/* 글리프는 문자 코드 순으로 정렬되어 있다 */
		auto Iter = std::lower_bound(m_vecGlyph.begin(), m_vecGlyph.end(), static_cast<uint32_t>(wcCharacter),
			[](const Glyph& tGlyph, uint32_t iChar) { return tGlyph.Character < iChar; });
		if (Iter != m_vecGlyph.end() && Iter->Character == static_cast<uint32_t>(wcCharacter))
			return &(*Iter);

		if (static_cast<uint32_t>(wcCharacter) != m_iDefaultCharacter && m_iDefaultCharacter != 0)
			return Find_Glyph(static_cast<wchar_t>(m_iDefaultCharacter));
		return nullptr;
	}

	void XM_CALLCONV SpriteFont::DrawString(SpriteBatch* pBatch, const wchar_t* pText, const XMFLOAT2& vPosition, FXMVECTOR vColor,
		float fRotation, const XMFLOAT2& vOrigin, float fScale) const
	{
		DrawString(pBatch, pText, vPosition, vColor, fRotation, vOrigin, XMFLOAT2(fScale, fScale));
	}

	void XM_CALLCONV SpriteFont::DrawString(SpriteBatch* pBatch, const wchar_t* pText, const XMFLOAT2& vPosition, FXMVECTOR vColor,
		float fRotation, const XMFLOAT2& vOrigin, const XMFLOAT2& vScale) const
	{
		if (pBatch == nullptr || pText == nullptr || m_pTexture == nullptr)
			return;

		Engine::ShaderResourceView* pSRV = m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		float fX = 0.f, fY = 0.f;
		for (const wchar_t* p = pText; *p != 0; ++p)
		{
			const wchar_t wc = *p;
			if (wc == L'\r') continue;
			if (wc == L'\n') { fX = 0.f; fY += m_fLineSpacing; continue; }

			const Glyph* pGlyph = Find_Glyph(wc);
			if (pGlyph == nullptr) continue;

			fX += pGlyph->XOffset;
			if (fX < 0.f) fX = 0.f;

			const float fGlyphW = static_cast<float>(pGlyph->SubrectRight - pGlyph->SubrectLeft);
			if (!std::iswspace(static_cast<wint_t>(wc)))
			{
				RECT tRect{};
				tRect.left = pGlyph->SubrectLeft;
				tRect.top = pGlyph->SubrectTop;
				tRect.right = pGlyph->SubrectRight;
				tRect.bottom = pGlyph->SubrectBottom;
				/* 글자 위치를 origin 으로 표현하면 회전 / 스케일이 문자열 전체 기준으로 적용된다 */
				const XMFLOAT2 vGlyphOrigin{ vOrigin.x - fX, vOrigin.y - (fY + pGlyph->YOffset) };
				pBatch->Draw(pSRV, vPosition, &tRect, vColor, fRotation, vGlyphOrigin, vScale);
			}

			fX += fGlyphW + pGlyph->XAdvance;
		}
	}

	XMVECTOR XM_CALLCONV SpriteFont::MeasureString(const wchar_t* pText) const
	{
		float fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (pText != nullptr)
		{
			for (const wchar_t* p = pText; *p != 0; ++p)
			{
				const wchar_t wc = *p;
				if (wc == L'\r') continue;
				if (wc == L'\n') { fX = 0.f; fY += m_fLineSpacing; continue; }
				const Glyph* pGlyph = Find_Glyph(wc);
				if (pGlyph == nullptr) continue;

				fX += pGlyph->XOffset;
				if (fX < 0.f) fX = 0.f;
				const float fGlyphW = static_cast<float>(pGlyph->SubrectRight - pGlyph->SubrectLeft);
				const float fGlyphH = static_cast<float>(pGlyph->SubrectBottom - pGlyph->SubrectTop) + pGlyph->YOffset;
				fWidth = (std::max)(fWidth, fX + fGlyphW);
				fHeight = (std::max)(fHeight, fY + (std::max)(m_fLineSpacing, fGlyphH));
				fX += fGlyphW + pGlyph->XAdvance;
			}
		}
		return XMVectorSet(fWidth, fHeight, 0.f, 0.f);
	}
}
