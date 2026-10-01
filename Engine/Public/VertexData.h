#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

template<class T>
struct  VertexDataBase
{
public:
	static const vector<INPUT_ELEMENT_DESC>& GetLayoutDesc() { return InputElement_Desc; }
private:
	static vector<INPUT_ELEMENT_DESC> InputElement_Desc;
};

struct  VertexTextureData : public VertexDataBase<VertexTextureData>
{
	Float3 Position;
	//Color color;
	Float2 UV;
};

vector<INPUT_ELEMENT_DESC>	VertexDataBase<VertexTextureData>::InputElement_Desc =
{  
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA , 0 },
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12 , INPUT_PER_VERTEX_DATA , 0 }
};

struct  VertexColorData : public VertexDataBase<VertexColorData>
{
	Float3 Position = { 0.f,0.f, 0.f, };
	Color  Color = { 0.f,0.f,0.f,0.f };
};

vector<INPUT_ELEMENT_DESC>	VertexDataBase<VertexColorData>::InputElement_Desc =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA , 0 },
	{"COLOR", 0, Diligent::TEX_FORMAT_RGBA32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , INPUT_PER_VERTEX_DATA , 0 }
};


NS_END