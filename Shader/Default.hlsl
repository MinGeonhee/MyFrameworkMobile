// row_major 붙이는이유 : GPU에서 기본값으로는  열을 기준으로 모시기를 하기떄문에 row_major 를 붙여줘야지 행 기준으로 계산을 실행한다

#define MAX_LIGHTS 50
/* Direct 에서 최대로 사용할수 있는 ShaderResourceView는 128개까지 이론상 사용가능하지만 여러 제한사항으로 다 사용하기엔 벅차다.. */
/*   */ 
#define MAX_DIFFUSETEX 2
#define MAX_MTLTEXTURE 6

#define DIFFUSE     0
#define AMBIENT     1
#define NORMALMAP   2
#define SPECULAR    3
#define EMISSIVE    4
#define METAL       5



row_major matrix g_matWorld : register(b0) =
{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
};

row_major matrix g_matCanvas : register(b4) =
{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
};


struct CameraInfo
{
    float  fStartX;
    float  fStartY;
    float  fWidth;
    float  fHeight;
    float  fFar;
    float  fNear;
    float  fFov;
    float  fCamIndex;      // A-plan : which camera drew this pixel
};

cbuffer g_CameraBuffer : register(b1)
{
    
    row_major matrix g_matView =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    row_major matrix g_matProj =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    row_major matrix g_matViewProj =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    row_major matrix g_matViewInv =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    row_major matrix g_matProjInv =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    // 카메라의 Near , Far , Start X , Start Y , Width, Hegith
    float4      g_CamPosition = { 0.f, 0.f, 0.f, 1.f };
    
    CameraInfo  g_tCameraInfo = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
}




struct Light_Buffer_Data
{
    float4 vLightDiffuse;
    float4 vLightAmbient;
    float4 vLightSpecular;
    float4 vLightDir;
    float4 vPosition;
    
    //아마 점광원전용?
    float   fRange;
    /* float3 Dummy → float 3개 : Vulkan(SPIR-V std140) 에서도 C++ 구조체(LIGHT_BUF_DATA)와 같은 96 Byte 를 유지하기 위함 */
    float   fDummy0;
    float   fDummy1;
    float   fDummy2;
};


cbuffer g_LightGroupBuffer : register(b3)
{
    Light_Buffer_Data         tGlobalLightBuffer;
    Light_Buffer_Data         arrPointLightBuffer[MAX_LIGHTS];
    int                       iPointLightNum;
    int3                      iDummy;
}

cbuffer g_MouseBuffer : register(b4)
{
    float3 vMouseWorldPosition = {0.f,0.f,0.f};
    bool isMouseOn = false;
};

SamplerState        g_SamplerState : register(s0);
 
/* 그냥 기본적인 Texture 느낌? */
Texture2D           g_Texture  : register(t0);
/*  뭔가 텍스처를 합치거나 할때 쓰면될듯?  */ 
Texture2D           g_MixTextureArr[MAX_DIFFUSETEX];
Texture2D           g_MaskTexture;
Texture2D           g_MaterialTexture[MAX_MTLTEXTURE];


Texture2D           g_Textures[128] : register(t30);

Texture2DArray      g_TextureArray;

float               g_UIAlpha = 1.f;
float               g_DT = 0.f;


float4              g_Color = { 1.f, 1.f , 1.f , 1.f };
float               g_Ratio = 0.f;

/* Render State를 정의해보자 */ 

sampler DefaultSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = wrap;
    AddressV = wrap;
};

sampler PointSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = wrap;
    AddressV = wrap;
};

sampler Wrap_Sampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = wrap;
    AddressV = wrap;
};

sampler ClampSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = clamp; // 가로축 고정
    AddressV = clamp; // 세로축 고정
};

sampler MirrorSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror; // 가로축 고정
    AddressV = Mirror; // 세로축 고정
};

sampler MirrorOnceSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror_Once; // 가로축 고정
    AddressV = Mirror_Once; // 세로축 고정
};

sampler BorderSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror; // 가로축 고정
    AddressV = Mirror; // 세로축 고정
};



RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = BACK;
    ScissorEnable = false;
};




RasterizerState RS_NONCULL
{
    FillMode = Solid;
    CullMode = NONE;
    ScissorEnable = false;
};

RasterizerState RS_CULLFRONT
{
    FillMode = Solid;
    CullMode = FRONT;
    ScissorEnable = false;
};

RasterizerState RS_WIRE_NONCULL
{
    FillMode = WIREFRAME;
    CullMode = NONE;
    ScissorEnable = false;
};

RasterizerState RS_WIRE_CULLBACK
{
    FillMode = WIREFRAME;
    CullMode = BACK;
    ScissorEnable = false;
};

RasterizerState RS_WIRE_CULLFRONT
{
    FillMode = WIREFRAME;
    CullMode = FRONT;
    ScissorEnable = false;
};

RasterizerState RS_Sky
{
    FillMode = Solid;
    CullMode = FRONT;
    ScissorEnable = false;
};


DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = LESS_EQUAL;
};


DepthStencilState DSS_Sky
{
    DepthEnable = false;
    DepthWriteMask = zero;
    //DepthFunc = ;
};

DepthStencilState DSS_NONE
{
    DepthEnable = false;
    DepthWriteMask = zero;
    //DepthFunc = ;
};


BlendState BS_Default
{
    BlendEnable[0] = false;
};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
};

BlendState BS_AddBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend       = One;
    DestBlend      = One;
    BlendOp        = add;
};


float Get_Shade(vector vLightDir, vector vNormal)
{
    return max(dot(normalize(vLightDir) * -1.f, normalize(vNormal)),0.f);
}

float3 ComputePointLight(
    Light_Buffer_Data light,
    float3 worldPos,
    float3 normal,
    float3 albedo)
{
    // 광원 → 픽셀 방향 벡터
    float3 toLight = light.vPosition.xyz - worldPos;
    float dist = length(toLight);
    toLight = normalize(toLight);

    // 감쇠(attenuation)
    // range를 넘으면 빛이 사라지게
    float atten = saturate(1.0f - (dist / light.fRange));
    atten *= atten; // 부드러운 감쇠

    // Diffuse = (N·L)
    float NdotL = saturate(dot(normal, toLight));

    float3 diffuse = albedo * light.vLightDiffuse.xyz * NdotL * atten;
    float3 ambient = albedo * light.vLightAmbient.xyz * 0.3f; // 임의 환경광

    return diffuse + ambient;
}


