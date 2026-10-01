#include "CVIBuffer.h"
#include "CGameObject.h"
#include "CGraphicDevice.h"

namespace
{
	/* -------------------------------------------------------------------------------------- */
	/* D3D11 Dynamic 버퍼는 Map(WRITE_NO_OVERWRITE) 시 이전 내용이 남아 있어 읽고 고쳐 쓸 수 있다. */
	/* Diligent(Vulkan) Dynamic 버퍼는 매 프레임 새 메모리를 받으므로 CPU 사본을 들고 있다가      */
	/* Unmap / Draw 시점에 통째로 올린다.                                                        */
	/* -------------------------------------------------------------------------------------- */
	struct DYNAMIC_SHADOW
	{
		std::vector<uint8>		vecData;
		uint64					iUploadedFrame{ ~0ull };
		bool					isDirty{ true };
	};

	std::mutex											g_DynamicMutex;
	std::unordered_map<Diligent::IBuffer*, DYNAMIC_SHADOW>	g_umapDynamicShadow;

	void Upload_Dynamic(DeviceContext* pContext, Diligent::IBuffer* pBuffer, DYNAMIC_SHADOW& tShadow)
	{
		if (pContext == nullptr || pBuffer == nullptr || tShadow.vecData.empty())
			return;

		void* pMapped = nullptr;
		pContext->MapBuffer(pBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pMapped);
		if (pMapped == nullptr)
			return;
		::memcpy(pMapped, tShadow.vecData.data(), tShadow.vecData.size());
		pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);

		CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
		tShadow.iUploadedFrame = pGraphicDevice != nullptr ? pGraphicDevice->Get_FrameIndex() : 0;
		tShadow.isDirty = false;
	}

	void Ensure_Dynamic(DeviceContext* pContext, Diligent::IBuffer* pBuffer)
	{
		if (pBuffer == nullptr || pBuffer->GetDesc().Usage != Diligent::USAGE_DYNAMIC)
			return;

		std::lock_guard<std::mutex> Lock(g_DynamicMutex);
		auto Iter = g_umapDynamicShadow.find(pBuffer);
		if (Iter == g_umapDynamicShadow.end())
			return;

		CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
		const uint64 iFrame = pGraphicDevice != nullptr ? pGraphicDevice->Get_FrameIndex() : 0;
		if (Iter->second.isDirty || Iter->second.iUploadedFrame != iFrame)
			Upload_Dynamic(pContext, pBuffer, Iter->second);
	}
}

CVIBuffer::CVIBuffer(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CComponent(pDevice, pContext),m_pVertexPosition(nullptr)  , m_iCountVertexBuffer(0)
	, m_pVertexBuffer(nullptr), m_pIndexBuffer(nullptr) , m_iOffsetIndex(0) , m_iOffsetVertex(0) ,
	m_iCountIndex(0) , m_iCountVertex(0)  , m_iStrideVertex(0) , m_iStrideIndex(0) , m_ePrimitiveType(TOPOLOGY_TRIANGLELIST)
	, m_pIndices(nullptr) , m_eIndexForamat(INDEX_FORMAT::END) , m_pNormal(nullptr)
{
}

CVIBuffer::CVIBuffer(const CVIBuffer& Prototype)
	: CComponent(Prototype),m_pVertexPosition(Prototype.m_pVertexPosition) , m_pVertexBuffer(Prototype.m_pVertexBuffer), m_pIndexBuffer(Prototype.m_pIndexBuffer), m_iCountVertexBuffer(Prototype.m_iCountVertexBuffer)
	,m_iCountVertex(Prototype.m_iCountVertex),m_iCountIndex(Prototype.m_iCountIndex) , m_iStrideVertex(Prototype.m_iStrideVertex), m_iStrideIndex(Prototype.m_iStrideIndex)
	, m_iOffsetIndex(Prototype.m_iOffsetIndex), m_iOffsetVertex(Prototype.m_iOffsetVertex) , m_ePrimitiveType(Prototype.m_ePrimitiveType)
	, m_pIndices(Prototype.m_pIndices) , m_eIndexForamat(Prototype.m_eIndexForamat)
	, m_pNormal(Prototype.m_pNormal)
{
}

HRESULT CVIBuffer::Ready_VertexBuffer(const BUFFER_DESC& tVertexDesc, const SUBRESOURCE_DATA& tVertexData, uint32 iOffsetIndex)
{
	NULLPTR_RETURN_EFAIL(m_pDevice); NULLPTR_RETURN_EFAIL(m_pDeviceContext);

	/* Vertex Buffer Create */
	if (FAILED(Create_Buffer(tVertexDesc, &tVertexData, &m_pVertexBuffer)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer::Ready_IndexBuffer(const BUFFER_DESC& tIndexDesc, const SUBRESOURCE_DATA& tIndexData, uint32 iOffsetIndex)
{
	NULLPTR_RETURN_EFAIL(m_pDevice); NULLPTR_RETURN_EFAIL(m_pDeviceContext);

	/* Index Buffer Create */
	if (FAILED(Create_Buffer(tIndexDesc, &tIndexData, &m_pIndexBuffer)))
		return E_FAIL;

	/* 피킹용 Index 사본 (원본은 기존 m_pIndices 를 덮어썼다 : 이전 메모리는 각 VIBuffer 가 직접 해제한다) */
	if (m_iStrideIndex == sizeof(uint16))
	{
		m_eIndexForamat = INDEX_FORMAT::UINT16;
		m_pIndices = static_cast<void*>(new uint16[m_iCountIndex]);
		memcpy(m_pIndices, tIndexData.pSysMem, sizeof(uint16)*m_iCountIndex);
	}
	else if (m_iStrideIndex == sizeof(uint32))
	{
		m_eIndexForamat = INDEX_FORMAT::UINT32;
		m_pIndices = static_cast<void*>(new uint32[m_iCountIndex]);
		memcpy(m_pIndices, tIndexData.pSysMem, sizeof(uint32) * m_iCountIndex);
	}

	return S_OK;
}

HRESULT CVIBuffer::Bind_Resource()
{
	IASetVertexBuffers(0, m_iCountVertexBuffer, m_pVertexBuffer.GetAddressOf(), &m_iStrideVertex, &m_iOffsetVertex);
	IASetIndexBuffer(m_pIndexBuffer.Get(), m_iStrideIndex == 2 ? INDEX_TYPE_UINT16 : INDEX_TYPE_UINT32, m_iOffsetIndex);
	IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT	CVIBuffer::Draw()
{
	Context_DrawIndexed(m_iCountIndex, 0, 0);

	return S_OK;
}

#pragma region D3D11 DeviceContext 호출 대응

HRESULT CVIBuffer::Create_Buffer(const BUFFER_DESC& tDesc, const SUBRESOURCE_DATA* pInitialData, ComPtr<Buffer>* ppBuffer)
{
	NULLPTR_RETURN_EFAIL(m_pDevice);
	NULLPTR_RETURN_EFAIL(ppBuffer);

	Diligent::BufferDesc tBufferDesc{};
	tBufferDesc.Name = "CVIBuffer";
	tBufferDesc.Size = tDesc.ByteWidth;
	tBufferDesc.ElementByteStride = tDesc.StructureByteStride;

	if (tDesc.BindFlags & BUFFER_BIND_VERTEX_BUFFER)	tBufferDesc.BindFlags |= Diligent::BIND_VERTEX_BUFFER;
	if (tDesc.BindFlags & BUFFER_BIND_INDEX_BUFFER)		tBufferDesc.BindFlags |= Diligent::BIND_INDEX_BUFFER;
	if (tDesc.BindFlags & BUFFER_BIND_CONSTANT_BUFFER)	tBufferDesc.BindFlags |= Diligent::BIND_UNIFORM_BUFFER;
	if (tDesc.BindFlags & BUFFER_BIND_SHADER_RESOURCE)	tBufferDesc.BindFlags |= Diligent::BIND_SHADER_RESOURCE;

	const bool isDynamic = tDesc.Usage == BUFFER_USAGE_DYNAMIC;
	if (isDynamic)
	{
		tBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
		tBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	}
	else if (tDesc.Usage == BUFFER_USAGE_IMMUTABLE)
		tBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
	else
		tBufferDesc.Usage = Diligent::USAGE_DEFAULT;

	/* Diligent Dynamic 버퍼는 생성시 초기값을 받을 수 없다 → CPU 사본에 넣어두고 첫 사용때 올린다 */
	Diligent::BufferData tData{};
	Diligent::BufferData* pData = nullptr;
	if (!isDynamic && pInitialData != nullptr && pInitialData->pSysMem != nullptr)
	{
		tData.pData = pInitialData->pSysMem;
		tData.DataSize = tDesc.ByteWidth;
		pData = &tData;
	}

	m_pDevice->CreateBuffer(tBufferDesc, pData, ppBuffer->ReleaseAndGetAddressOf());
	if (ppBuffer->Get() == nullptr)
		return E_FAIL;

	if (isDynamic)
	{
		std::lock_guard<std::mutex> Lock(g_DynamicMutex);
		DYNAMIC_SHADOW& tShadow = g_umapDynamicShadow[ppBuffer->Get()];
		tShadow.vecData.assign(tDesc.ByteWidth, 0);
		if (pInitialData != nullptr && pInitialData->pSysMem != nullptr)
			::memcpy(tShadow.vecData.data(), pInitialData->pSysMem, tDesc.ByteWidth);
		tShadow.isDirty = true;
	}

	return S_OK;
}

HRESULT CVIBuffer::Map(Buffer* pBuffer, uint32 iSubResource, MAP_MODE eMapMode, uint32 iMapFlags, MAPPED_SUBRESOURCE* pMapped)
{
	NULLPTR_RETURN_EFAIL(pBuffer);
	NULLPTR_RETURN_EFAIL(pMapped);

	std::lock_guard<std::mutex> Lock(g_DynamicMutex);
	auto Iter = g_umapDynamicShadow.find(pBuffer);
	if (Iter == g_umapDynamicShadow.end())
		return E_FAIL;

	pMapped->pData = Iter->second.vecData.data();
	pMapped->RowPitch = static_cast<uint32>(Iter->second.vecData.size());
	pMapped->DepthPitch = pMapped->RowPitch;
	return S_OK;
}

void CVIBuffer::Unmap(Buffer* pBuffer, uint32 iSubResource)
{
	std::lock_guard<std::mutex> Lock(g_DynamicMutex);
	auto Iter = g_umapDynamicShadow.find(pBuffer);
	if (Iter == g_umapDynamicShadow.end())
		return;

	Iter->second.isDirty = true;
	Upload_Dynamic(m_pDeviceContext.Get(), pBuffer, Iter->second);
}

void CVIBuffer::UpdateSubresource(Buffer* pBuffer, uint32 iSubResource, const void* pBox, const void* pData, uint32 iRowPitch, uint32 iDepthPitch)
{
	if (pBuffer == nullptr || pData == nullptr || m_pDeviceContext == nullptr)
		return;

	m_pDeviceContext->UpdateBuffer(pBuffer, 0, pBuffer->GetDesc().Size, pData, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void CVIBuffer::Release_DynamicBuffer(Buffer* pBuffer)
{
	std::lock_guard<std::mutex> Lock(g_DynamicMutex);
	g_umapDynamicShadow.erase(pBuffer);
}

void CVIBuffer::IASetVertexBuffers(uint32 iStartSlot, uint32 iNumBuffers, Buffer* const* ppBuffers, const uint32* pStrides, const uint32* pOffsets)
{
	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	if (pGraphicDevice == nullptr)
		return;

	uint64 iOffsets[2]{ 0, 0 };
	for (uint32 i = 0; i < iNumBuffers && i < 2; ++i)
	{
		iOffsets[i] = pOffsets != nullptr ? pOffsets[i] : 0;
		Ensure_Dynamic(m_pDeviceContext.Get(), ppBuffers[i]);
	}

	pGraphicDevice->IA_SetVertexBuffers(iStartSlot, iNumBuffers, ppBuffers, pStrides, iOffsets);
}

void CVIBuffer::IASetIndexBuffer(Buffer* pIndexBuffer, INDEX_TYPE eIndexType, uint32 iOffset)
{
	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	if (pGraphicDevice == nullptr)
		return;

	pGraphicDevice->IA_SetIndexBuffer(pIndexBuffer, eIndexType == INDEX_TYPE_UINT16 ? Diligent::VT_UINT16 : Diligent::VT_UINT32, iOffset);
}

void CVIBuffer::IASetPrimitiveTopology(PRIMITIVE_TOPOLOGY eTopology)
{
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
		pGraphicDevice->IA_SetPrimitiveTopology(eTopology);
}

void CVIBuffer::Context_DrawIndexed(uint32 iIndexCount, uint32 iStartIndex, int32 iBaseVertex)
{
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
		pGraphicDevice->DrawIndexed(iIndexCount, iStartIndex, iBaseVertex);
}

void CVIBuffer::Context_Draw(uint32 iVertexCount, uint32 iStartVertex)
{
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
		pGraphicDevice->Draw(iVertexCount, iStartVertex);
}

void CVIBuffer::Context_DrawInstanced(uint32 iVertexCountPerInstance, uint32 iInstanceCount, uint32 iStartVertex, uint32 iStartInstance)
{
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
		pGraphicDevice->DrawInstanced(iVertexCountPerInstance, iInstanceCount, iStartVertex, iStartInstance);
}

void CVIBuffer::Context_DrawIndexedInstanced(uint32 iIndexCountPerInstance, uint32 iInstanceCount, uint32 iStartIndex, int32 iBaseVertex, uint32 iStartInstance)
{
	if (CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance())
		pGraphicDevice->DrawIndexedInstanced(iIndexCountPerInstance, iInstanceCount, iStartIndex, iBaseVertex, iStartInstance);
}

#pragma endregion

bool CVIBuffer::Picking_Buffer(_In_ const RAY_INFO& tLocalRayInfo, _Outptr_ RAY_HIT* pLocalRayHit)
{
	/* Local상태의 RayInfo를 받아서  RayHit를 받고 Out시켜준다.. */
	if (m_iCountIndex <= 3) return false;
	if (pLocalRayHit == nullptr) return false;

	Vector4 RayPos = XMLoadFloat3(&tLocalRayInfo.vRayOrigin);
	Vector4 RayDir = XMLoadFloat3(&tLocalRayInfo.vRayDir);

	if (m_eIndexForamat == INDEX_FORMAT::UINT16)
	{
		uint16* pIndices = static_cast<uint16*>(m_pIndices);

		for (uint32 i = 0;i < m_iCountIndex; i+=3)
		{
			Vector4 vPos1 = XMLoadFloat3(&m_pVertexPosition[pIndices[i]]);
			Vector4 vPos2 = XMLoadFloat3(&m_pVertexPosition[pIndices[i+1]]);
			Vector4 vPos3 = XMLoadFloat3(&m_pVertexPosition[pIndices[i+2]]);

			float fDist{0.f};

			if (TriangleTests::Intersects(RayPos, RayDir, vPos1, vPos2, vPos3, fDist) == true)
			{
				uint32 iTriIndex[3] = { pIndices[i], pIndices[i + 1], pIndices[i + 2] };
				pLocalRayHit->SetRayHit(tLocalRayInfo,fDist,vPos1,vPos2,vPos3,iTriIndex);

				if (m_pNormal != nullptr)
					pLocalRayHit->Set_Normal(m_pNormal[pIndices[i]], m_pNormal[pIndices[i+1]], m_pNormal[pIndices[i+2]]);

				pLocalRayHit->pRayHitGameObject = static_cast<CGameObject*>(this->Get_Owner());
				return true;
			}
		}
	}
	else
	{
		uint32* pIndices = static_cast<uint32*>(m_pIndices);

		for (uint32 i = 0;i < m_iCountIndex;i += 3)
		{
			Vector4 vPos1 = XMLoadFloat3(&m_pVertexPosition[pIndices[i]]);
			Vector4 vPos2 = XMLoadFloat3(&m_pVertexPosition[pIndices[i + 1]]);
			Vector4 vPos3 = XMLoadFloat3(&m_pVertexPosition[pIndices[i + 2]]);

			float fDist{ 0.f };

			if (TriangleTests::Intersects( RayPos , RayDir , vPos1, vPos2, vPos3, fDist) == true)
			{
				pLocalRayHit->SetRayHit(tLocalRayInfo,fDist,vPos1,vPos2,vPos3,&pIndices[i]);

				if (m_pNormal != nullptr)
					pLocalRayHit->Set_Normal(m_pNormal[pIndices[i]], m_pNormal[pIndices[i + 1]], m_pNormal[pIndices[i + 2]]);

				pLocalRayHit->pRayHitGameObject = static_cast<CGameObject*>(this->Get_Owner());
				return true;
			}
		}
	}

	return false;
}

HRESULT	CVIBuffer::Proto_Initialize(const BUFFER_DESC& tVertexDesc, const SUBRESOURCE_DATA& tVertexData, const BUFFER_DESC& tIndexDesc, const SUBRESOURCE_DATA& tIndexData, PRIMITIVE_TOPOLOGY ePrimitiveType, uint32 iOffsetVertex, uint32 iOffsetIndex)
{
	m_ePrimitiveType = ePrimitiveType;

	if (FAILED(Ready_VertexBuffer(tVertexDesc, tVertexData, iOffsetVertex)))
		return E_FAIL;

	if (FAILED(Ready_IndexBuffer(tIndexDesc, tIndexData, iOffsetIndex )))
		return E_FAIL;

	return S_OK;
}

HRESULT	CVIBuffer::Clone_Initialize(void* pArg)
{
	if (FAILED(__super::Clone_Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}
