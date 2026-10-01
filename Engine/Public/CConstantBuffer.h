#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

/* --------------------------------------------------------------------------------------------- */
/* 원본 : D3D11_USAGE_DYNAMIC 상수버퍼를 Map(WRITE_DISCARD) 로 갱신했다.                            */
/* 현재 : Diligent USAGE_DYNAMIC 버퍼를 사용한다.                                                    */
/*        Vulkan 의 Dynamic 버퍼는 "그 프레임 안에서 Map 된 적이 있어야" 사용할 수 있으므로              */
/*        CPU 사본을 들고 있다가 Draw 직전에 이번 프레임에 아직 올리지 않았다면 다시 Map 해준다.        */
/* --------------------------------------------------------------------------------------------- */
class ENGINE_DLL CConstantBufferBase : public CBase
{
protected:
	explicit CConstantBufferBase(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CConstantBufferBase() {}
protected:
	HRESULT				Initialize_Buffer(uint32 iByteSize, const char* pName);
public:
	/* CPU 사본에 값을 쓴다 (GPU 업로드는 Ensure_Uploaded 에서 한다) */
	HRESULT				Write(const void* pData, uint32 iSize, uint32 iByteOffset = 0);
	/* 이번 프레임에 아직 올라가지 않았거나 값이 바뀌었다면 GPU 버퍼를 갱신한다 */
	HRESULT				Ensure_Uploaded();
	/* 즉시 업로드 (원본 CopyData 의 Map/Unmap 시점과 같다) */
	HRESULT				Upload_Now();

	Buffer*				Get_Buffer() const { return m_pBuffer.Get(); }
	uint32				Get_ByteSize() const { return static_cast<uint32>(m_vecData.size()); }
	const uint8*		Get_Data() const { return m_vecData.data(); }
	uint8*				Get_Data() { return m_vecData.data(); }
protected:
	ComPtr<Buffer>			m_pBuffer;
	vector<uint8>			m_vecData;
	bool					m_isDirty;
	uint64					m_iUploadedFrame;
protected:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	static CConstantBufferBase* Create(uint32 iByteSize, const char* pName, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};


template<typename T>
class  CConstantBuffer final:
    public CConstantBufferBase
{
private:
	explicit	CConstantBuffer(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
		: CConstantBufferBase(pDevice, pContext)
	{
	}
	virtual	~CConstantBuffer() {}
private:
	HRESULT	Initialize()
	{
		return Initialize_Buffer(static_cast<uint32>(sizeof(T)), "CConstantBuffer");
	}
public:
	ConstantBuffer* GetConstantBufferPtr()  const{ return m_pBuffer.Get(); }

	HRESULT	CopyData(const T& tData)
	{
		if (FAILED(Write(&tData, static_cast<uint32>(sizeof(T)), 0)))
			return E_FAIL;

		return Upload_Now();
	}

public:
	static CConstantBuffer* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	{
		CConstantBuffer* pConstantBuffer = new CConstantBuffer(pDevice, pContext);

		if (FAILED(pConstantBuffer->Initialize()))
		{
			Safe_Release(pConstantBuffer);
			MSG_BOX("ConstantBuffer is failed to Create");
			return nullptr;
		}

		return pConstantBuffer;
	}
	virtual void Free()
	{
		CConstantBufferBase::Free();

		/* CConstantBuffer::Free()*/
	}
};



NS_END
