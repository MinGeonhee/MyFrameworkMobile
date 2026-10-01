#include "Engine_Define.h"

#ifdef _DEBUG

#include "imgui_impl_diligent.h"
#include "CGraphicDevice.h"

#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"

namespace
{
	constexpr const char* g_pImGuiShader = R"(
cbuffer ImGuiConstants
{
    float4x4 g_matProjection;
};

Texture2D    Texture;
SamplerState Texture_sampler;

struct VS_IN
{
    float2 vPosition : ATTRIB0;
    float2 vUV       : ATTRIB1;
    float4 vColor    : ATTRIB2;
};

struct PS_IN
{
    float4 vPosition : SV_Position;
    float4 vColor    : COLOR0;
    float2 vUV       : TEXCOORD0;
};

PS_IN VSMain(VS_IN In)
{
    PS_IN Out;
    Out.vPosition = mul(g_matProjection, float4(In.vPosition, 0.f, 1.f));
    Out.vColor = In.vColor;
    Out.vUV = In.vUV;
    return Out;
}

float4 PSMain(PS_IN In) : SV_Target
{
    return In.vColor * Texture.Sample(Texture_sampler, In.vUV);
}
)";

	struct PSO_ENTRY
	{
		uint32										iNumRT = 0;
		Diligent::TEXTURE_FORMAT					eRTFormat = Diligent::TEX_FORMAT_UNKNOWN;
		Diligent::TEXTURE_FORMAT					eDSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
		Engine::ComPtr<Diligent::IPipelineState>	pPSO;
		Engine::ComPtr<Diligent::IShaderResourceBinding> pSRB;
	};

	struct BACKEND_TEXTURE
	{
		Engine::ComPtr<Diligent::ITexture>	pTexture;
	};

	struct BACKEND_DATA
	{
		Engine::ComPtr<Diligent::IRenderDevice>	pDevice;
		Engine::ComPtr<Diligent::IDeviceContext>	pContext;
		Engine::ComPtr<Diligent::IShader>		pVS;
		Engine::ComPtr<Diligent::IShader>		pPS;
		Engine::ComPtr<Diligent::IBuffer>		pCB;
		Engine::ComPtr<Diligent::IBuffer>		pVB;
		Engine::ComPtr<Diligent::IBuffer>		pIB;
		uint32									iVBSize = 0;
		uint32									iIBSize = 0;
		std::vector<PSO_ENTRY>					vecPSO;
	};

	BACKEND_DATA* g_pBackend = nullptr;

	PSO_ENTRY* Get_PSO(const Engine::GRAPHIC_PIPELINE_STATE& tState)
	{
		const Diligent::TEXTURE_FORMAT eRT = tState.iNumRenderTargets > 0 ? tState.eRTVFormats[0] : Diligent::TEX_FORMAT_UNKNOWN;
		for (PSO_ENTRY& tEntry : g_pBackend->vecPSO)
			if (tEntry.iNumRT == tState.iNumRenderTargets && tEntry.eRTFormat == eRT && tEntry.eDSVFormat == tState.eDSVFormat)
				return &tEntry;

		Diligent::GraphicsPipelineStateCreateInfo tCI{};
		tCI.PSODesc.Name = "ImGui PSO";
		tCI.GraphicsPipeline.NumRenderTargets = static_cast<Diligent::Uint8>(tState.iNumRenderTargets);
		tCI.GraphicsPipeline.RTVFormats[0] = eRT;
		tCI.GraphicsPipeline.DSVFormat = tState.eDSVFormat;
		tCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		tCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		tCI.GraphicsPipeline.RasterizerDesc.ScissorEnable = true;
		tCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

		Diligent::RenderTargetBlendDesc& tBlend = tCI.GraphicsPipeline.BlendDesc.RenderTargets[0];
		tBlend.BlendEnable = true;
		tBlend.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
		tBlend.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		tBlend.BlendOp = Diligent::BLEND_OPERATION_ADD;
		tBlend.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
		tBlend.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		tBlend.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;

		const Diligent::LayoutElement arrLayout[] = {
			Diligent::LayoutElement{ 0, 0, 2, Diligent::VT_FLOAT32, false },
			Diligent::LayoutElement{ 1, 0, 2, Diligent::VT_FLOAT32, false },
			Diligent::LayoutElement{ 2, 0, 4, Diligent::VT_UINT8, true },
		};
		tCI.GraphicsPipeline.InputLayout.LayoutElements = arrLayout;
		tCI.GraphicsPipeline.InputLayout.NumElements = 3;
		tCI.pVS = g_pBackend->pVS.Get();
		tCI.pPS = g_pBackend->pPS.Get();

		Diligent::SamplerDesc tSampler{};
		tSampler.MinFilter = tSampler.MagFilter = tSampler.MipFilter = Diligent::FILTER_TYPE_LINEAR;
		tSampler.AddressU = tSampler.AddressV = tSampler.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
		Diligent::ImmutableSamplerDesc tImmutable{ Diligent::SHADER_TYPE_PIXEL, "Texture", tSampler };
		Diligent::ShaderResourceVariableDesc tVariable{ Diligent::SHADER_TYPE_PIXEL, "Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC };
		tCI.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		tCI.PSODesc.ResourceLayout.Variables = &tVariable;
		tCI.PSODesc.ResourceLayout.NumVariables = 1;
		tCI.PSODesc.ResourceLayout.ImmutableSamplers = &tImmutable;
		tCI.PSODesc.ResourceLayout.NumImmutableSamplers = 1;

		PSO_ENTRY tEntry{};
		tEntry.iNumRT = tState.iNumRenderTargets;
		tEntry.eRTFormat = eRT;
		tEntry.eDSVFormat = tState.eDSVFormat;
		g_pBackend->pDevice->CreateGraphicsPipelineState(tCI, tEntry.pPSO.ReleaseAndGetAddressOf());
		if (tEntry.pPSO == nullptr)
			return nullptr;
		if (auto* pVar = tEntry.pPSO->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ImGuiConstants"))
			pVar->Set(g_pBackend->pCB.Get());
		tEntry.pPSO->CreateShaderResourceBinding(tEntry.pSRB.ReleaseAndGetAddressOf(), true);

		g_pBackend->vecPSO.push_back(tEntry);
		return &g_pBackend->vecPSO.back();
	}

	void Destroy_Texture(ImTextureData* pTex)
	{
		BACKEND_TEXTURE* pBackendTex = static_cast<BACKEND_TEXTURE*>(pTex->BackendUserData);
		if (pBackendTex != nullptr)
			IM_DELETE(pBackendTex);
		pTex->SetTexID(ImTextureID_Invalid);
		pTex->SetStatus(ImTextureStatus_Destroyed);
		pTex->BackendUserData = nullptr;
	}

	void Update_Texture(ImTextureData* pTex)
	{
		if (pTex->Status == ImTextureStatus_WantCreate)
		{
			IM_ASSERT(pTex->Format == ImTextureFormat_RGBA32);
			BACKEND_TEXTURE* pBackendTex = IM_NEW(BACKEND_TEXTURE)();

			Diligent::TextureDesc tDesc{};
			tDesc.Name = "ImGui Texture";
			tDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			tDesc.Width = static_cast<Diligent::Uint32>(pTex->Width);
			tDesc.Height = static_cast<Diligent::Uint32>(pTex->Height);
			tDesc.MipLevels = 1;
			tDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
			tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			tDesc.Usage = Diligent::USAGE_DEFAULT;

			Diligent::TextureSubResData tSub{ pTex->GetPixels(), static_cast<Diligent::Uint64>(pTex->GetPitch()) };
			Diligent::TextureData tData{ &tSub, 1 };
			g_pBackend->pDevice->CreateTexture(tDesc, &tData, pBackendTex->pTexture.ReleaseAndGetAddressOf());

			pTex->SetTexID(reinterpret_cast<ImTextureID>(pBackendTex->pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE)));
			pTex->SetStatus(ImTextureStatus_OK);
			pTex->BackendUserData = pBackendTex;
		}
		else if (pTex->Status == ImTextureStatus_WantUpdates)
		{
			BACKEND_TEXTURE* pBackendTex = static_cast<BACKEND_TEXTURE*>(pTex->BackendUserData);
			for (ImTextureRect& tRect : pTex->Updates)
			{
				Diligent::Box tBox{ tRect.x, static_cast<Diligent::Uint32>(tRect.x + tRect.w), tRect.y, static_cast<Diligent::Uint32>(tRect.y + tRect.h) };
				Diligent::TextureSubResData tSub{ pTex->GetPixelsAt(tRect.x, tRect.y), static_cast<Diligent::Uint64>(pTex->GetPitch()) };
				g_pBackend->pContext->UpdateTexture(pBackendTex->pTexture.Get(), 0, 0, tBox, tSub,
					Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
			pTex->SetStatus(ImTextureStatus_OK);
		}
		if (pTex->Status == ImTextureStatus_WantDestroy && pTex->UnusedFrames > 0)
			Destroy_Texture(pTex);
	}

	bool Create_DeviceObjects()
	{
		if (g_pBackend->pVS != nullptr)
			return true;

		Diligent::ShaderCreateInfo tCI{};
		tCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		tCI.Source = g_pImGuiShader;
		tCI.Desc.UseCombinedTextureSamplers = true;
		tCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		tCI.Desc.Name = "ImGui VS";
		tCI.EntryPoint = "VSMain";
		g_pBackend->pDevice->CreateShader(tCI, g_pBackend->pVS.ReleaseAndGetAddressOf());
		tCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		tCI.Desc.Name = "ImGui PS";
		tCI.EntryPoint = "PSMain";
		g_pBackend->pDevice->CreateShader(tCI, g_pBackend->pPS.ReleaseAndGetAddressOf());

		Diligent::BufferDesc tCBDesc{};
		tCBDesc.Name = "ImGui CB";
		tCBDesc.Size = sizeof(float) * 16;
		tCBDesc.Usage = Diligent::USAGE_DYNAMIC;
		tCBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		tCBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		g_pBackend->pDevice->CreateBuffer(tCBDesc, nullptr, g_pBackend->pCB.ReleaseAndGetAddressOf());

		return g_pBackend->pVS != nullptr && g_pBackend->pPS != nullptr && g_pBackend->pCB != nullptr;
	}
}

bool ImGui_ImplDiligent_Init(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pContext)
{
	ImGuiIO& io = ImGui::GetIO();
	IMGUI_CHECKVERSION();
	IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

	g_pBackend = IM_NEW(BACKEND_DATA)();
	g_pBackend->pDevice = pDevice;
	g_pBackend->pContext = pContext;

	io.BackendRendererUserData = g_pBackend;
	io.BackendRendererName = "imgui_impl_diligent";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Renderer_TextureMaxWidth = platform_io.Renderer_TextureMaxHeight = 4096;
	return true;
}

void ImGui_ImplDiligent_Shutdown()
{
	if (g_pBackend == nullptr)
		return;

	for (ImTextureData* pTex : ImGui::GetPlatformIO().Textures)
		if (pTex->RefCount == 1)
			Destroy_Texture(pTex);

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = nullptr;
	io.BackendRendererUserData = nullptr;
	io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
	IM_DELETE(g_pBackend);
	g_pBackend = nullptr;
}

void ImGui_ImplDiligent_NewFrame()
{
	if (g_pBackend != nullptr)
		Create_DeviceObjects();
}

void ImGui_ImplDiligent_RenderDrawData(ImDrawData* pDrawData)
{
	if (g_pBackend == nullptr || pDrawData == nullptr || pDrawData->DisplaySize.x <= 0.f || pDrawData->DisplaySize.y <= 0.f)
		return;
	if (!Create_DeviceObjects())
		return;

	if (pDrawData->Textures != nullptr)
		for (ImTextureData* pTex : *pDrawData->Textures)
			if (pTex->Status != ImTextureStatus_OK)
				Update_Texture(pTex);

	if (pDrawData->TotalVtxCount <= 0)
		return;

	Diligent::IDeviceContext* pContext = g_pBackend->pContext.Get();
	Diligent::IRenderDevice* pDevice = g_pBackend->pDevice.Get();

	/* 버퍼 크기 확보 */
	if (g_pBackend->pVB == nullptr || g_pBackend->iVBSize < static_cast<uint32>(pDrawData->TotalVtxCount))
	{
		g_pBackend->iVBSize = pDrawData->TotalVtxCount + 5000;
		Diligent::BufferDesc tDesc{};
		tDesc.Name = "ImGui VB";
		tDesc.Size = g_pBackend->iVBSize * sizeof(ImDrawVert);
		tDesc.Usage = Diligent::USAGE_DYNAMIC;
		tDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(tDesc, nullptr, g_pBackend->pVB.ReleaseAndGetAddressOf());
	}
	if (g_pBackend->pIB == nullptr || g_pBackend->iIBSize < static_cast<uint32>(pDrawData->TotalIdxCount))
	{
		g_pBackend->iIBSize = pDrawData->TotalIdxCount + 10000;
		Diligent::BufferDesc tDesc{};
		tDesc.Name = "ImGui IB";
		tDesc.Size = g_pBackend->iIBSize * sizeof(ImDrawIdx);
		tDesc.Usage = Diligent::USAGE_DYNAMIC;
		tDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
		tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(tDesc, nullptr, g_pBackend->pIB.ReleaseAndGetAddressOf());
	}
	if (g_pBackend->pVB == nullptr || g_pBackend->pIB == nullptr)
		return;

	{
		void* pVtx = nullptr;
		void* pIdx = nullptr;
		pContext->MapBuffer(g_pBackend->pVB.Get(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pVtx);
		pContext->MapBuffer(g_pBackend->pIB.Get(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pIdx);
		ImDrawVert* pVtxDst = static_cast<ImDrawVert*>(pVtx);
		ImDrawIdx* pIdxDst = static_cast<ImDrawIdx*>(pIdx);
		for (const ImDrawList* pList : pDrawData->CmdLists)
		{
			memcpy(pVtxDst, pList->VtxBuffer.Data, pList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(pIdxDst, pList->IdxBuffer.Data, pList->IdxBuffer.Size * sizeof(ImDrawIdx));
			pVtxDst += pList->VtxBuffer.Size;
			pIdxDst += pList->IdxBuffer.Size;
		}
		pContext->UnmapBuffer(g_pBackend->pVB.Get(), Diligent::MAP_WRITE);
		pContext->UnmapBuffer(g_pBackend->pIB.Get(), Diligent::MAP_WRITE);
	}

	/* 직교 투영 (column-major , mul(M , v)) */
	{
		const float L = pDrawData->DisplayPos.x;
		const float R = pDrawData->DisplayPos.x + pDrawData->DisplaySize.x;
		const float T = pDrawData->DisplayPos.y;
		const float B = pDrawData->DisplayPos.y + pDrawData->DisplaySize.y;
		const float matProj[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f, 0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f, 0.0f },
			{ 0.0f,         0.0f,           0.5f, 0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f, 1.0f },
		};
		void* pMapped = nullptr;
		pContext->MapBuffer(g_pBackend->pCB.Get(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pMapped);
		memcpy(pMapped, matProj, sizeof(matProj));
		pContext->UnmapBuffer(g_pBackend->pCB.Get(), Diligent::MAP_WRITE);
	}

	Engine::CGraphicDevice* pGraphicDevice = Engine::CGraphicDevice::Get_Instance();
	if (pGraphicDevice == nullptr)
		return;
	const Engine::GRAPHIC_PIPELINE_STATE& tState = pGraphicDevice->Get_PipelineState();
	PSO_ENTRY* pPSO = Get_PSO(tState);
	if (pPSO == nullptr)
		return;

	const Diligent::TextureDesc& tRTDesc = pGraphicDevice->GetRTV()->GetTexture()->GetDesc();
	Diligent::Viewport tVP{ pDrawData->DisplaySize.x * pDrawData->FramebufferScale.x, pDrawData->DisplaySize.y * pDrawData->FramebufferScale.y };
	pContext->SetViewports(1, &tVP, tRTDesc.Width, tRTDesc.Height);

	Diligent::IBuffer* pVB = g_pBackend->pVB.Get();
	const Diligent::Uint64 iOffset = 0;
	pContext->SetVertexBuffers(0, 1, &pVB, &iOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
	pContext->SetIndexBuffer(g_pBackend->pIB.Get(), 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	pContext->SetPipelineState(pPSO->pPSO.Get());

	Diligent::IShaderResourceVariable* pTextureVar = pPSO->pSRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Texture");

	int iGlobalIdxOffset = 0;
	int iGlobalVtxOffset = 0;
	const ImVec2 vClipOff = pDrawData->DisplayPos;
	const ImVec2 vClipScale = pDrawData->FramebufferScale;
	for (const ImDrawList* pList : pDrawData->CmdLists)
	{
		for (int iCmd = 0; iCmd < pList->CmdBuffer.Size; ++iCmd)
		{
			const ImDrawCmd* pCmd = &pList->CmdBuffer[iCmd];
			if (pCmd->UserCallback != nullptr)
			{
				if (pCmd->UserCallback != ImDrawCallback_ResetRenderState)
					pCmd->UserCallback(pList, pCmd);
				continue;
			}

			ImVec2 vClipMin((pCmd->ClipRect.x - vClipOff.x) * vClipScale.x, (pCmd->ClipRect.y - vClipOff.y) * vClipScale.y);
			ImVec2 vClipMax((pCmd->ClipRect.z - vClipOff.x) * vClipScale.x, (pCmd->ClipRect.w - vClipOff.y) * vClipScale.y);
			vClipMin.x = (std::max)(vClipMin.x, 0.f);
			vClipMin.y = (std::max)(vClipMin.y, 0.f);
			vClipMax.x = (std::min)(vClipMax.x, static_cast<float>(tRTDesc.Width));
			vClipMax.y = (std::min)(vClipMax.y, static_cast<float>(tRTDesc.Height));
			if (vClipMax.x <= vClipMin.x || vClipMax.y <= vClipMin.y)
				continue;

			const Diligent::Rect tScissor{ static_cast<Diligent::Int32>(vClipMin.x), static_cast<Diligent::Int32>(vClipMin.y),
				static_cast<Diligent::Int32>(vClipMax.x), static_cast<Diligent::Int32>(vClipMax.y) };
			pContext->SetScissorRects(1, &tScissor, tRTDesc.Width, tRTDesc.Height);

			Diligent::ITextureView* pTextureView = reinterpret_cast<Diligent::ITextureView*>(pCmd->GetTexID());
			if (pTextureVar != nullptr && pTextureView != nullptr)
				pTextureVar->Set(pTextureView);
			pContext->CommitShaderResources(pPSO->pSRB.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			Diligent::DrawIndexedAttribs tDraw{};
			tDraw.NumIndices = pCmd->ElemCount;
			tDraw.IndexType = sizeof(ImDrawIdx) == 2 ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
			tDraw.FirstIndexLocation = pCmd->IdxOffset + iGlobalIdxOffset;
			tDraw.BaseVertex = pCmd->VtxOffset + iGlobalVtxOffset;
			tDraw.Flags = Diligent::DRAW_FLAG_VERIFY_STATES;
			pContext->DrawIndexed(tDraw);
		}
		iGlobalIdxOffset += pList->IdxBuffer.Size;
		iGlobalVtxOffset += pList->VtxBuffer.Size;
	}

	/* 엔진 상태 복구 */
	pGraphicDevice->Invalidate_BoundState();
}

#endif
