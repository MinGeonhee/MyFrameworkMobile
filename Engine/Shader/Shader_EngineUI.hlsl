#include "../../Shader/Default.hlsl"


struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD0;
};




VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    vector vPosition = mul(vector(In.vPosition, 1.f),g_matWorld);
    vPosition = mul(vPosition,g_matCanvas);
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

float4 PS_MAIN(VS_OUT In) : SV_Target
{
    float4 Color = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    Color.w = Color.w * g_UIAlpha;
    
    if (Color.a < 0.05f)
        discard;
    
    
    return Color;
}

float4 PS_BACKGROUND(VS_OUT In) : SV_Target
{
    float4 Color = { 1.f, 0.f, 0.f, 0.f };
    return Color;
}

technique11 DefaultTechnique
{ 

    pass Default
    {
        VertexShader = compile  vs_5_0 VS_MAIN();
        PixelShader  = compile  ps_5_0 PS_MAIN();
    } 

}