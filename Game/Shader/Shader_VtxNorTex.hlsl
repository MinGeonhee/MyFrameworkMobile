#include "../../Shader/Default.hlsl"


vector g_vMtrlAmbient = vector(0.3f, 0.3f, 0.3f, 1.f);
vector g_vMtrlSpecular = vector(1.f, 1.f, 1.f, 1.f);

struct VS_IN
{
    float3 vPosition    : POSITION;
    float3 vNormal      : NORMAL;
    float2 vTexcoord    : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vNormal   : NORMAL;
    float2 vTexcoord : TEXCOORD0; 
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos  : TEXCOORD2;
};
// VS_OUT /  PS_IN 구조체는 동일해야한다./

struct PS_IN
{
    float4 vPosition : SV_Position;
    float4 vNormal   : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos  : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse     : SV_TARGET0;
    float4 vNormal      : SV_TARGET1;
    float4 vDepth       : SV_TARGET2;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    vector vPosition = mul(vector(In.vPosition, 1.f), g_matWorld);
    vPosition = mul(vPosition, g_matView);
    vPosition = mul(vPosition, g_matProj);
    
    Out.vPosition   = vPosition;
    Out.vTexcoord   = In.vTexcoord;
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_matWorld));
    Out.vWorldPos = mul(vector(In.vPosition , 1.f) , g_matWorld);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    vector vSourDiffuse = g_MixTextureArr[0].Sample(DefaultSampler, In.vTexcoord * 6.f);
    vector vDestDiffuse = g_MixTextureArr[1].Sample(DefaultSampler, In.vTexcoord * 6.f);
    vector vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vSourDiffuse * (1 - vMask.r) + vDestDiffuse * vMask.r;
    Out.vNormal  = (In.vNormal.xyz * 0.5 + 0.5, 0.f);  
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w , In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);

    
    return Out;
}

technique11 DefaultTechnique
{

    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader  = compile  ps_5_0 PS_MAIN();
    }
}

