#include "CConstantBuffer.h"
#include "CGraphicDevice.h"

#include "Graphics/GraphicsEngine/interface/DeviceContext.h"

CConstantBufferBase::CConstantBufferBase(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext), m_pBuffer(nullptr), m_vecData{}, m_isDirty(true), m_iUploadedFrame(~0ull)
{
}

HRESULT CConstantBufferBase::Initialize_Buffer(uint32 iByteSize, const char* pName)
{
	if (m_pDevice == nullptr || iByteSize == 0)
		return E_FAIL;

	/* 상수버퍼는 16 Byte 단위 */
	const uint32 iAlignedSize = (iByteSize + 15u) & ~15u;
	m_vecData.assign(iAlignedSize, 0);

	Diligent::BufferDesc tDesc{};
	tDesc.Name = pName;
	tDesc.Size = iAlignedSize;
	tDesc.Usage = Diligent::USAGE_DYNAMIC;
	tDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

	m_pDevice->CreateBuffer(tDesc, nullptr, &m_pBuffer);
	if (m_pBuffer == nullptr)
		return E_FAIL;

	m_isDirty = true;
	return S_OK;
}

HRESULT CConstantBufferBase::Write(const void* pData, uint32 iSize, uint32 iByteOffset)
{
	if (pData == nullptr || iByteOffset + iSize > m_vecData.size())
		return E_FAIL;

	::memcpy(m_vecData.data() + iByteOffset, pData, iSize);
	m_isDirty = true;
	return S_OK;
}

HRESULT CConstantBufferBase::Ensure_Uploaded()
{
	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	const uint64 iFrame = pGraphicDevice != nullptr ? pGraphicDevice->Get_FrameIndex() : 0;

	if (!m_isDirty && m_iUploadedFrame == iFrame)
		return S_OK;

	return Upload_Now();
}

HRESULT CConstantBufferBase::Upload_Now()
{
	if (m_pBuffer == nullptr || m_pDeviceContext == nullptr)
		return E_FAIL;

	void* pMapped = nullptr;
	m_pDeviceContext->MapBuffer(m_pBuffer.Get(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pMapped);
	if (pMapped == nullptr)
		return E_FAIL;

	::memcpy(pMapped, m_vecData.data(), m_vecData.size());
	m_pDeviceContext->UnmapBuffer(m_pBuffer.Get(), Diligent::MAP_WRITE);

	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	m_iUploadedFrame = pGraphicDevice != nullptr ? pGraphicDevice->Get_FrameIndex() : 0;
	m_isDirty = false;
	return S_OK;
}

CConstantBufferBase* CConstantBufferBase::Create(uint32 iByteSize, const char* pName, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CConstantBufferBase* pInstance = new CConstantBufferBase(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Buffer(iByteSize, pName)))
	{
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CConstantBufferBase::Free()
{
	__super::Free();

	m_pBuffer.Reset();
	m_pDevice.Reset();
	m_pDeviceContext.Reset();
}
