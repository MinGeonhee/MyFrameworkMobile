#include "DirectXTK_Compat.h"
#include "CGraphicDevice.h"

#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"

namespace
{
	constexpr const char* g_pBatchShader = R"(
cbuffer BatchConstants
{
    row_major float4x4 g_matWVP;
};

struct VS_IN
{
    float3 vPosition : ATTRIB0;
    float4 vColor    : ATTRIB1;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vColor    : COLOR0;
};

VS_OUT VSMain(VS_IN In)
{
    VS_OUT Out;
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_matWVP);
    Out.vColor = In.vColor;
    return Out;
}

float4 PSMain(VS_OUT In) : SV_Target
{
    return In.vColor;
}
)";

	struct BATCH_PSO
	{
		Diligent::PRIMITIVE_TOPOLOGY				eTopology;
		uint32										iNumRT;
		Diligent::TEXTURE_FORMAT					eRTFormats[8];
		Diligent::TEXTURE_FORMAT					eDSVFormat;
		Engine::ComPtr<Diligent::IPipelineState>	pPSO;
		Engine::ComPtr<Diligent::IShaderResourceBinding> pSRB;
	};

	struct BATCH_RESOURCES
	{
		Engine::ComPtr<Diligent::IShader>	pVS;
		Engine::ComPtr<Diligent::IShader>	pPS;
		Engine::ComPtr<Diligent::IBuffer>	pVB;
		Engine::ComPtr<Diligent::IBuffer>	pIB;
		Engine::ComPtr<Diligent::IBuffer>	pCB;
		std::vector<BATCH_PSO>				vecPSO;
		uint32								iVBCapacity = 0;
		uint32								iIBCapacity = 0;
	};

	BATCH_RESOURCES g_tBatch;
	DirectX::BasicEffect* g_pCurrentEffect = nullptr;

	bool Ensure_Buffer(Engine::Device* pDevice, Engine::ComPtr<Diligent::IBuffer>& pBuffer, uint32& iCapacity, uint32 iSize, Diligent::BIND_FLAGS eBind, const char* pName)
	{
		if (pBuffer != nullptr && iCapacity >= iSize)
			return true;

		iCapacity = (std::max)(iSize, iCapacity * 2u);
		Diligent::BufferDesc tDesc{};
		tDesc.Name = pName;
		tDesc.Size = iCapacity;
		tDesc.Usage = Diligent::USAGE_DYNAMIC;
		tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		tDesc.BindFlags = eBind;
		pBuffer.Reset();
		pDevice->CreateBuffer(tDesc, nullptr, pBuffer.ReleaseAndGetAddressOf());
		return pBuffer != nullptr;
	}

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

	BATCH_PSO* Find_PSO(Engine::Device* pDevice, Diligent::PRIMITIVE_TOPOLOGY eTopology, const Engine::GRAPHIC_PIPELINE_STATE& tState)
	{
		for (BATCH_PSO& tPSO : g_tBatch.vecPSO)
		{
			if (tPSO.eTopology != eTopology || tPSO.iNumRT != tState.iNumRenderTargets || tPSO.eDSVFormat != tState.eDSVFormat)
				continue;
			bool isSame = true;
			for (uint32 i = 0; i < tPSO.iNumRT; ++i)
				isSame &= tPSO.eRTFormats[i] == tState.eRTVFormats[i];
			if (isSame)
				return &tPSO;
		}

		if (g_tBatch.pVS == nullptr)
		{
			Diligent::ShaderCreateInfo tCI{};
			tCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
			tCI.Source = g_pBatchShader;
			tCI.Desc.UseCombinedTextureSamplers = true;
			tCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			tCI.Desc.Name = "PrimitiveBatch VS";
			tCI.EntryPoint = "VSMain";
			pDevice->CreateShader(tCI, g_tBatch.pVS.ReleaseAndGetAddressOf());
			tCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			tCI.Desc.Name = "PrimitiveBatch PS";
			tCI.EntryPoint = "PSMain";
			pDevice->CreateShader(tCI, g_tBatch.pPS.ReleaseAndGetAddressOf());
			if (g_tBatch.pVS == nullptr || g_tBatch.pPS == nullptr)
				return nullptr;
		}

		Diligent::GraphicsPipelineStateCreateInfo tCI{};
		tCI.PSODesc.Name = "PrimitiveBatch PSO";
		tCI.GraphicsPipeline.NumRenderTargets = static_cast<Diligent::Uint8>(tState.iNumRenderTargets);
		for (uint32 i = 0; i < tState.iNumRenderTargets; ++i)
			tCI.GraphicsPipeline.RTVFormats[i] = tState.eRTVFormats[i];
		tCI.GraphicsPipeline.DSVFormat = tState.eDSVFormat;
		tCI.GraphicsPipeline.PrimitiveTopology = eTopology;
		tCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		tCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = tState.eDSVFormat != Diligent::TEX_FORMAT_UNKNOWN;
		tCI.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;
		tCI.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;

		const Diligent::LayoutElement arrLayout[] = {
			Diligent::LayoutElement{ 0, 0, 3, Diligent::VT_FLOAT32, false },
			Diligent::LayoutElement{ 1, 0, 4, Diligent::VT_FLOAT32, false },
		};
		tCI.GraphicsPipeline.InputLayout.LayoutElements = arrLayout;
		tCI.GraphicsPipeline.InputLayout.NumElements = 2;
		tCI.pVS = g_tBatch.pVS.Get();
		tCI.pPS = g_tBatch.pPS.Get();
		tCI.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		BATCH_PSO tPSO{};
		tPSO.eTopology = eTopology;
		tPSO.iNumRT = tState.iNumRenderTargets;
		for (uint32 i = 0; i < 8; ++i)
			tPSO.eRTFormats[i] = tState.eRTVFormats[i];
		tPSO.eDSVFormat = tState.eDSVFormat;
		pDevice->CreateGraphicsPipelineState(tCI, tPSO.pPSO.ReleaseAndGetAddressOf());
		if (tPSO.pPSO == nullptr)
			return nullptr;

		if (auto* pVar = tPSO.pPSO->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "BatchConstants"))
			pVar->Set(g_tBatch.pCB.Get());
		tPSO.pPSO->CreateShaderResourceBinding(tPSO.pSRB.ReleaseAndGetAddressOf(), true);

		g_tBatch.vecPSO.push_back(tPSO);
		return &g_tBatch.vecPSO.back();
	}
}

namespace DirectX
{
	void BasicEffect::Apply(Engine::DeviceContext* pContext)
	{
		g_pCurrentEffect = this;
	}

	XMFLOAT4X4 BasicEffect::Get_WorldViewProj() const
	{
		XMFLOAT4X4 matWVP;
		XMStoreFloat4x4(&matWVP, XMLoadFloat4x4(&m_matWorld) * XMLoadFloat4x4(&m_matView) * XMLoadFloat4x4(&m_matProj));
		return matWVP;
	}

	BasicEffect* BasicEffect::Get_Current()
	{
		return g_pCurrentEffect;
	}

	void PrimitiveBatch_Draw(Engine::PRIMITIVE_TOPOLOGY eTopology, const VertexPositionColor* pVertices, size_t iVertexCount, const uint16_t* pIndices, size_t iIndexCount)
	{
		Engine::CGraphicDevice* pGraphicDevice = Engine::CGraphicDevice::Get_Instance();
		if (pGraphicDevice == nullptr || pVertices == nullptr || iVertexCount == 0)
			return;

		Engine::Device* pDevice = pGraphicDevice->Get_Device().Get();
		Engine::DeviceContext* pContext = pGraphicDevice->Get_DeviceContext().Get();

		const uint32 iVBSize = static_cast<uint32>(sizeof(VertexPositionColor) * iVertexCount);
		const uint32 iIBSize = static_cast<uint32>(sizeof(uint16_t) * iIndexCount);
		uint32 iCBCapacity = g_tBatch.pCB != nullptr ? 64u : 0u;
		if (!Ensure_Buffer(pDevice, g_tBatch.pCB, iCBCapacity, sizeof(XMFLOAT4X4), Diligent::BIND_UNIFORM_BUFFER, "PrimitiveBatch CB") ||
			!Ensure_Buffer(pDevice, g_tBatch.pVB, g_tBatch.iVBCapacity, iVBSize, Diligent::BIND_VERTEX_BUFFER, "PrimitiveBatch VB"))
			return;
		if (iIndexCount > 0 && !Ensure_Buffer(pDevice, g_tBatch.pIB, g_tBatch.iIBCapacity, iIBSize, Diligent::BIND_INDEX_BUFFER, "PrimitiveBatch IB"))
			return;

		BATCH_PSO* pPSO = Find_PSO(pDevice, eTopology, pGraphicDevice->Get_PipelineState());
		if (pPSO == nullptr)
			return;

		const XMFLOAT4X4 matWVP = g_pCurrentEffect != nullptr ? g_pCurrentEffect->Get_WorldViewProj() : XMFLOAT4X4{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		Upload(pContext, g_tBatch.pCB.Get(), &matWVP, sizeof(matWVP));
		Upload(pContext, g_tBatch.pVB.Get(), pVertices, iVBSize);

		Diligent::IBuffer* pVB = g_tBatch.pVB.Get();
		const Diligent::Uint64 iOffset = 0;
		pContext->SetVertexBuffers(0, 1, &pVB, &iOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
		pContext->SetPipelineState(pPSO->pPSO.Get());
		pContext->CommitShaderResources(pPSO->pSRB.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		if (iIndexCount > 0)
		{
			Upload(pContext, g_tBatch.pIB.Get(), pIndices, iIBSize);
			pContext->SetIndexBuffer(g_tBatch.pIB.Get(), 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			Diligent::DrawIndexedAttribs tDraw{};
			tDraw.NumIndices = static_cast<Diligent::Uint32>(iIndexCount);
			tDraw.IndexType = Diligent::VT_UINT16;
			tDraw.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
			pContext->DrawIndexed(tDraw);
		}
		else
		{
			Diligent::DrawAttribs tDraw{};
			tDraw.NumVertices = static_cast<Diligent::Uint32>(iVertexCount);
			tDraw.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
			pContext->Draw(tDraw);
		}

		/* 엔진 상태머신의 PSO / 정점버퍼 캐시를 무효화한다 */
		pGraphicDevice->Invalidate_BoundState();
	}

	void SpriteBatch_ReleaseResources();

	void PrimitiveBatch_ReleaseResources()
	{
		g_tBatch = BATCH_RESOURCES{};
		g_pCurrentEffect = nullptr;
		SpriteBatch_ReleaseResources();
	}
}
