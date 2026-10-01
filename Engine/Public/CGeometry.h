#pragma once
#include "CBase.h"
#include "VertexData.h"


NS_BEGIN(Engine)

/* 아마 T가 들어온다면 VertexDataBase관련일것 */


template <class T>
class  CGeometry :    public CBase
{
private:
	explicit CGeometry() {};
	virtual ~CGeometry() = default;
private:
	HRESULT	Initialize();
public:
	//T의 Size 반환
	uint32 GetTypeSize() { return static_cast<uint32>(sizeof(T)); }
	//버퍼 구조체의 Layout 정보를 담고있는 Vector를 반환시켜준다
	const vector<INPUT_ELEMENT_DESC>& GetLayoutDesc() { return T::GetLayoutDesc(); }
public:
	const vector<T>& GetVertices() { return m_vecVertices; }
	const vector<uint32>& GetIndices() { return m_vecIndiecs; }

	uint32 GetVertexCount() { return static_cast<uint32>(m_vecVertices.size()); }
	uint32 GetIndexCount() { return static_cast<uint32>(m_vecIndiecs.size()); }

	void	Add_Vertex(const T& Vertex) { m_vecVertices.push_back(Vertex); };
	void	Add_Vertices(const vector<T>& Vertices) { m_vecVertices.insert(m_vecVertices.end(), Vertices.begin(), Vertices.end()); }
	void	Set_Vertex(const vector<T>& Vertices) { m_vecVertices = Vertices; }

	void	Add_Index(const uint32& Index) { m_vecIndiecs.push_back(Index); }
	void	Add_Indices(const vector<uint32> Indices) { m_vecIndiecs.insert(m_vecIndiecs.end(), Indices.begin(), Indices.end()); }
	void	Set_Indices(const vector<uint32> Indices) { m_vecIndiecs = Indices; }
private:
	vector<T>			m_vecVertices;
	vector<uint32>		m_vecIndiecs;
public:
	static CGeometry* Create()
	{
		CGeometry* pGeometry = new CGeometry();
		
		if (pGeometry == nullptr)
		{
			MSG_BOX("Geometry is failed to Create");
			Safe_Release(pGeometry);
			return nullptr;
		}

		return pGeometry;
	}
	virtual void Free()
	{
		__super::Free();

		/* CGeometry::Free()*/
	}
};

NS_END