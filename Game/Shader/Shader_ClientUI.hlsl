#include "../../Shader/Default.hlsl"


float Loading_Progress = 0.f;


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

float4 PS_LOADING(VS_OUT In) : SV_Target
{
    float4 Color = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    
    if(In.vTexcoord.x > g_Ratio)
    {
        float fGray = (Color.r + Color.g + Color.b) / 3.0f;
        Color.rgb = float3(fGray, fGray, fGray);
    }
        
    
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
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile  vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader  = compile  ps_5_0 PS_MAIN();
    } 
    pass Loading
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LOADING();
    }

}