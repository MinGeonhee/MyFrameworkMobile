#pragma once
#include "CGeometry.h"

NS_BEGIN(Engine)


class ENGINE_DLL CGeometryHelper
{
public:
	static void	CreateRectangle(CGeometry<VertexColorData>* Geometry, Color color)
	{
		vector<VertexColorData> vertices;
		vector<uint32>			indices;
		vertices.resize(4);

		vertices[0].Position = Float3(-0.5f, +0.5f, 0);
		vertices[0].Color = color;


		vertices[1].Position = Float3(+0.5, +0.5f, 0);
		vertices[1].Color = color;

		vertices[2].Position = Float3(+0.5f, -0.5f, 0);
		vertices[2].Color = color;


		vertices[3].Position = Float3(-0.5f, -0.5f, 0);
		vertices[3].Color = color;

		indices = { 0,1,2,0,2,3 };

		Geometry->Set_Vertex(vertices);
		Geometry->Set_Indices(indices);
	}
	static void CreateRectangle(CGeometry<VertexTextureData>* Geometry)
	{

		vector<VertexTextureData> vertices;
		vector<uint32>			indices;
		vertices.resize(4);


		vertices[0].Position = Float3(-0.5f, +0.5f, 0);
		vertices[0].UV = { 0.f , 0.f };


		vertices[1].Position = Float3(+0.5, +0.5f, 0);
		vertices[1].UV = { 1.f , 0.f };

		vertices[2].Position = Float3(+0.5f, -0.5f, 0);
		vertices[2].UV = { 1.f , 1.f };


		vertices[3].Position = Float3(-0.5f, -0.5f, 0);
		vertices[3].UV = { 0.f , 1.f };

		indices = { 0,1,2,0,2,3 };

		Geometry->Set_Vertex(vertices);
		Geometry->Set_Indices(indices);
	}
};

NS_END

