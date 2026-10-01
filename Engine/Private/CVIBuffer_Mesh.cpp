#include "CVIBuffer_Mesh.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(const CVIBuffer_Mesh& ProtoType)
	: CVIBuffer(ProtoType)
{
}

HRESULT CVIBuffer_Mesh::Proto_Initialize()
{
	return S_OK;
}

HRESULT CVIBuffer_Mesh::Proto_Initialize_Anim(const VI_INFO& tVIInfo)
{
	/* Vertex , Index개수 */
	m_iCountVertex = tVIInfo.iVertexCnt;
	m_iCountIndex = tVIInfo.iIndexCnt;

	m_iCountVertexBuffer = 1;

	/* 정점 Offset */
	m_iOffsetIndex = 0;
	m_iOffsetVertex = 0;

	/* VertexStride 는 아래서 정의 */
	m_iStrideIndex = 4;


	if (FAILED(Ready_AnimBuffer(tVIInfo.strVertexDataPath, tVIInfo.strIndexDataPath)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Proto_Initialize_NonAnim(const VI_INFO& tVIInfo , const XMatrix_1& matTransform)
{
	/* Vertex , Index개수 */
	m_iCountVertex = tVIInfo.iVertexCnt;
	m_iCountIndex = tVIInfo.iIndexCnt;

	m_iCountVertexBuffer = 1;

	/* 정점 Offset */
	m_iOffsetIndex = 0;
	m_iOffsetVertex = 0;

	/* VertexStride 는 아래서 정의 */
	m_iStrideIndex = 4;


	if (FAILED(Ready_NonAnimBuffer(tVIInfo.strVertexDataPath, tVIInfo.strIndexDataPath, matTransform)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Clone_Initialize(void* pArg)
{


	return S_OK;
}

HRESULT CVIBuffer_Mesh::Ready_AnimBuffer(const string& VertexPath, const string& IndexPath)
{

	CFileStream VertexDataFile(VertexPath.c_str(), ios::binary);
	if (VertexDataFile.is_open() == false)
		return E_FAIL;

	CFileStream IndeXDataFile(IndexPath.c_str(), ios::binary);
	if (IndeXDataFile.is_open() == false)
		return E_FAIL;

	m_iStrideVertex = sizeof(VTXANIMMESH);


#pragma region VertexBuffer
	BUFFER_DESC tVertexDesc{};
	tVertexDesc.ByteWidth = m_iCountVertex * m_iStrideVertex;
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;
	tVertexDesc.CPUAccessFlags = 0;
	tVertexDesc.MiscFlags = 0;
	tVertexDesc.StructureByteStride = 0;

	SUBRESOURCE_DATA	tVertexData{};
	tVertexData.SysMemPitch = 0;
	tVertexData.SysMemSlicePitch = 0;

	VTXANIMMESH* pVertices = new VTXANIMMESH[m_iCountVertex];
	VertexDataFile.read(reinterpret_cast<char*>(&pVertices[0]), sizeof(VTXANIMMESH) * m_iCountVertex);
	tVertexData.pSysMem = pVertices;

	m_pVertexPosition = new XMFLOAT3[m_iCountVertex];
	m_pNormal = new XMFLOAT3[m_iCountVertex];

	for (uint32 i = 0; i < m_iCountVertex; ++i)
	{
		pVertices[i] = pVertices[i];
		m_pVertexPosition[i] = pVertices[i].vPosition;
		m_pNormal[i] = pVertices[i].vNormal;
	}


#pragma region Index Buffer

	BUFFER_DESC tIndexDesc{};
	tIndexDesc.ByteWidth = m_iCountIndex * m_iStrideIndex;
	tIndexDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;
	tIndexDesc.Usage = BUFFER_USAGE_DEFAULT;
	tIndexDesc.CPUAccessFlags = 0;
	tIndexDesc.MiscFlags = 0;
	tIndexDesc.StructureByteStride = 0;

	SUBRESOURCE_DATA	tIndexData{};
	tIndexData.SysMemPitch = 0;
	tIndexData.SysMemSlicePitch = 0;



	uint32* pIndices = new uint32[m_iCountIndex];

	IndeXDataFile.read(reinterpret_cast<char*>(&pIndices[0]), sizeof(uint32) * m_iCountIndex);
	tIndexData.pSysMem = pIndices;



#pragma endregion

	HRESULT hr = { E_FAIL };

	hr = CVIBuffer::Proto_Initialize(tVertexDesc, tVertexData, tIndexDesc, tIndexData, m_ePrimitiveType);


	if (hr == E_FAIL)
		return hr;


	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;


	return S_OK;




	return S_OK;
}

HRESULT CVIBuffer_Mesh::Ready_NonAnimBuffer(const string& VertexPath, const string& IndexPath, const XMatrix_1& matTransform)
{
	CFileStream VertexDataFile(VertexPath.c_str(), ios::binary);
	if (VertexDataFile.is_open() == false)
		return E_FAIL;

	CFileStream IndeXDataFile(IndexPath.c_str(), ios::binary);
	if (IndeXDataFile.is_open() == false)
		return E_FAIL;

	m_iStrideVertex = sizeof(VTXMESH);

#pragma region VertexBuffer
	BUFFER_DESC tVertexDesc{};
	tVertexDesc.ByteWidth = m_iCountVertex * m_iStrideVertex;
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;
	tVertexDesc.CPUAccessFlags = 0;
	tVertexDesc.MiscFlags = 0;
	tVertexDesc.StructureByteStride = 0;

	SUBRESOURCE_DATA	tVertexData{};
	tVertexData.SysMemPitch = 0;
	tVertexData.SysMemSlicePitch = 0;

	VTXMESH* pVertices = new VTXMESH[m_iCountVertex];
	//VertexDataFile.read(reinterpret_cast<char*>(&pVertices[0]), sizeof(VTXMESH) * m_iCountVertex);

	m_pVertexPosition = new XMFLOAT3[m_iCountVertex];
	m_pNormal = new XMFLOAT3[m_iCountVertex];

	for (uint32 i = 0; i < m_iCountVertex; ++i)
	{
		VertexDataFile.read(reinterpret_cast<char*>(&pVertices[i]), sizeof(VTXMESH));
		XMStoreFloat3(&pVertices[i].vPosition , XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition),matTransform));
		XMStoreFloat3(&pVertices[i].vNormal   ,	XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal),matTransform));
		XMStoreFloat3(&pVertices[i].vTangent  ,	XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent),matTransform));
		XMStoreFloat3(&pVertices[i].vBinormal , XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal),matTransform));
		m_pVertexPosition[i] = pVertices[i].vPosition;
		m_pNormal[i] = pVertices[i].vNormal;
	}

	tVertexData.pSysMem = pVertices;

	//m_pVertexPosition = new XMFLOAT3[m_iCountVertex];
	//m_pNormal = new XMFLOAT3[m_iCountVertex];

	//for (uint32 i = 0; i < m_iCountVertex; ++i)
	//{
	//	m_pVertexPosition[i] = pVertices[i].vPosition;

	//	m_pNormal[i] = pVertices[i].vNormal;
	//}

#pragma endregion


#pragma region Index Buffer

	BUFFER_DESC tIndexDesc{};
	tIndexDesc.ByteWidth = m_iCountIndex * m_iStrideIndex;
	tIndexDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;
	tIndexDesc.Usage = BUFFER_USAGE_DEFAULT;
	tIndexDesc.CPUAccessFlags = 0;
	tIndexDesc.MiscFlags = 0;
	tIndexDesc.StructureByteStride = 0;

	SUBRESOURCE_DATA	tIndexData{};
	tIndexData.SysMemPitch = 0;
	tIndexData.SysMemSlicePitch = 0;



	uint32* pIndices = new uint32[m_iCountIndex];

	IndeXDataFile.read(reinterpret_cast<char*>(&pIndices[0]), sizeof(uint32) * m_iCountIndex);
	tIndexData.pSysMem = pIndices;

#pragma endregion

	HRESULT hr = { E_FAIL };

	hr = CVIBuffer::Proto_Initialize(tVertexDesc, tVertexData, tIndexDesc, tIndexData, m_ePrimitiveType);


	if (hr == E_FAIL)
		return hr;


	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;


	return S_OK;
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_Mesh* pVIBuffer_Model = new CVIBuffer_Mesh(pDevice , pContext);

	if (FAILED(pVIBuffer_Model->Proto_Initialize()))
	{
		Safe_Release(pVIBuffer_Model);
		MSG_BOX("pVIBuffer_Model Proto is failed To Create");
		return nullptr;
	}

	return pVIBuffer_Model;
}


CVIBuffer_Mesh* CVIBuffer_Mesh::Create_NonAnim(const VI_INFO& tVIInfo, const XMatrix_1& matTransform, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_Mesh* pVIBuffer_Mesh = new CVIBuffer_Mesh(pDevice, pContext);

	if (FAILED(pVIBuffer_Mesh->Proto_Initialize_NonAnim(tVIInfo, matTransform)))
	{
		Safe_Release(pVIBuffer_Mesh);
		MSG_BOX("pVIBuffer_Model Proto is  failed To Create");
		return nullptr;
	}

	return pVIBuffer_Mesh;
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create_Anim(const VI_INFO& tVIInfo ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_Mesh* pVIBuffer_Mesh = new CVIBuffer_Mesh(pDevice, pContext);

	if (FAILED(pVIBuffer_Mesh->Proto_Initialize_Anim(tVIInfo)))
	{
		Safe_Release(pVIBuffer_Mesh);
		MSG_BOX("pVIBuffer_Model Proto is  failed To Create");
		return nullptr;
	}

	return pVIBuffer_Mesh;
}

CComponent* CVIBuffer_Mesh::Clone(void* pArg)
{
	CVIBuffer_Mesh* pVIBuffer_Mesh = new CVIBuffer_Mesh(*this);

	if (FAILED(pVIBuffer_Mesh->Clone_Initialize(pArg)))
	{
		Safe_Release(pVIBuffer_Mesh);
		MSG_BOX("pVIBuffer_Model Clone is failed To Create");
		return nullptr;
	}

	return pVIBuffer_Mesh;
}

void CVIBuffer_Mesh::Free()
{
	__super::Free();

	/* CVIBuffer_Model::Free() */
}

const INPUT_ELEMENT_DESC* CVIBuffer_Mesh::GetInputElementDesc()
{
	return VTXNORTEX().GetInputElementDesc().data();
}

uint32 CVIBuffer_Mesh::GetInputElementDescSize()
{
	return VTXNORTEX().GetInputElementDescSize();
}
