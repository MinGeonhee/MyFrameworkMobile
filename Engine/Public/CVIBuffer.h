#pragma once
#include "CComponent.h"

NS_BEGIN(Engine)


/* VIBUffer 생성의 이유 */
/* 물체를 Render하기위해 가장 기본이 되는 정점 Vertex 와 정점 Vertex의 배열 번호를 들고 있는 Index를 각 VertexBuffer , IndexBuffer를 이용하여 GPU RAM에 저장시켜주고 그걸 관리하는 Com객체를 들고있어 
   저장만 시키는거 뿐만아니라 Render과정에서 Draw과정을 당담한다.*/


/* D3D11_MAPPED_SUBRESOURCE / D3D11_MAP 대응 */
struct MAPPED_SUBRESOURCE
{
	void*		pData{ nullptr };
	uint32		RowPitch{ 0 };
	uint32		DepthPitch{ 0 };
};

enum MAP_MODE
{
	MAP_MODE_READ = 1,
	MAP_MODE_WRITE = 2,
	MAP_MODE_READ_WRITE = 3,
	MAP_MODE_WRITE_DISCARD = 4,
	MAP_MODE_WRITE_NO_OVERWRITE = 5,
};

enum INDEX_TYPE
{
	INDEX_TYPE_UINT16,
	INDEX_TYPE_UINT32,
};

class  ENGINE_DLL CVIBuffer abstract:  public CComponent
{
	enum class INDEX_FORMAT : uint8_t
	{
		UINT16,
		UINT32,
		END,
	};

protected:
	explicit CVIBuffer(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;
protected:	
	HRESULT							
		Proto_Initialize(
		const BUFFER_DESC& tVertexDesc, const SUBRESOURCE_DATA& tVertexData, const BUFFER_DESC& tIndexDesc, const SUBRESOURCE_DATA& tIndexData, PRIMITIVE_TOPOLOGY ePrimitiveType, uint32 iOffsetVertex = 0 , uint32 iOffsetIndex = 0
	);
	virtual HRESULT				Clone_Initialize(void* pArg);
private:
	HRESULT							Ready_VertexBuffer(const BUFFER_DESC& tVertexDesc, const SUBRESOURCE_DATA& tVertexData, uint32 iOffsetIndex);
	HRESULT							Ready_IndexBuffer(const BUFFER_DESC& tIndexDesc, const SUBRESOURCE_DATA& tIndexData ,uint32 iOffsetIndex);
public:
	virtual HRESULT							Bind_Resource();
	virtual HRESULT							Draw();

	void							Set_PrimitiveType(PRIMITIVE_TOPOLOGY eType) { m_ePrimitiveType = eType; }

#pragma region D3D11 DeviceContext 호출 대응 (내부에서 CGraphicDevice 상태머신 / Diligent 로 전달)
protected:
	HRESULT							Create_Buffer(const BUFFER_DESC& tDesc, const SUBRESOURCE_DATA* pInitialData, ComPtr<Buffer>* ppBuffer);
	HRESULT							Map(Buffer* pBuffer, uint32 iSubResource, MAP_MODE eMapMode, uint32 iMapFlags, MAPPED_SUBRESOURCE* pMapped);
	void							Unmap(Buffer* pBuffer, uint32 iSubResource);
	void							UpdateSubresource(Buffer* pBuffer, uint32 iSubResource, const void* pBox, const void* pData, uint32 iRowPitch, uint32 iDepthPitch);

	void							IASetVertexBuffers(uint32 iStartSlot, uint32 iNumBuffers, Buffer* const* ppBuffers, const uint32* pStrides, const uint32* pOffsets);
	void							IASetIndexBuffer(Buffer* pIndexBuffer, INDEX_TYPE eIndexType, uint32 iOffset);
	void							IASetPrimitiveTopology(PRIMITIVE_TOPOLOGY eTopology);
	void							Context_DrawIndexed(uint32 iIndexCount, uint32 iStartIndex, int32 iBaseVertex);
	void							Context_Draw(uint32 iVertexCount, uint32 iStartVertex);
	void							Context_DrawInstanced(uint32 iVertexCountPerInstance, uint32 iInstanceCount, uint32 iStartVertex, uint32 iStartInstance);
	void							Context_DrawIndexedInstanced(uint32 iIndexCountPerInstance, uint32 iInstanceCount, uint32 iStartIndex, int32 iBaseVertex, uint32 iStartInstance);
public:
	/* Dynamic 버퍼 CPU 사본 해제 (버퍼를 해제할 때 호출) */
	static void						Release_DynamicBuffer(Buffer* pBuffer);
#pragma endregion

public:
	bool							Picking_Buffer(_In_ const RAY_INFO& tLocalRayInfo, _Outptr_ RAY_HIT* pLocalRayHit);
protected:
	Float3*							m_pVertexPosition;
	void*							m_pIndices;
	Float3*							m_pNormal;
	INDEX_FORMAT					m_eIndexForamat;
	uint32							m_iCountVertexBuffer;
protected:
	/* Vertex 관련 */
	uint32							m_iStrideVertex;	// Vertex 구조체 하나의 Size
	uint32							m_iOffsetVertex;	// 메모리상 시작위치에 + offset해줄 변수
	uint32							m_iCountVertex;		// CountVertex 

	/* Index 관련 */
	uint32							m_iStrideIndex;		// IndeX도 마찬가지
	uint32							m_iOffsetIndex;
	uint32							m_iCountIndex;
protected:
	/* 그리는 방식 설정 */
	PRIMITIVE_TOPOLOGY		m_ePrimitiveType;
protected:
	/* Device : 정점 Create , DeviceContext -> 그리는 기능 및 세팅함수*/
	ComPtr<VertexBuffer>			m_pVertexBuffer;
	ComPtr<IndexBuffer>				m_pIndexBuffer;
public: 
	virtual const INPUT_ELEMENT_DESC* GetInputElementDesc() PURE;
	virtual uint32 GetInputElementDescSize() PURE;
	virtual CComponent* Clone(void*   pArg) PURE;
	virtual void	Free()
	{
		__super::Free();
		/* CVIBuffer::Free() */
		if (m_eMakeType == MAKE_TYPE::PROTOTYPE)
		{
			if (m_pVertexPosition != nullptr)
			{
				if (m_pVertexPosition != nullptr)
				{
					delete[] m_pVertexPosition;
					m_pVertexPosition = nullptr;
				}
				if (m_pIndices != nullptr)
				{
					delete[] static_cast<uint8*>(m_pIndices);
					m_pIndices = nullptr;
				}
				if (m_pNormal != nullptr)
				{
					delete[] m_pNormal;
					m_pNormal = nullptr;
				}
			}

			m_pVertexBuffer.Reset();
			m_pIndexBuffer.Reset();
		}
	}
};



NS_END
