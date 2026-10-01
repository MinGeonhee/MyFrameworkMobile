#include "../../Shader/Default.hlsl"


struct VS_IN
{
    float3 vPosition    : POSITION;
    float2 vTexcoord    : TEXCOORD0;
    float4 vRight       : TEXCOORD1;
    float4 vUp          : TEXCOORD2;
    float4 vLook        : TEXCOORD3;
    float4 VTranslation : TEXCOORD4;
    float2 vLifeTime    : TEXCOORD5;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
};


struct PS_OUT
{
    float4 vDiffuse : SV_Target0;  
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
 
    float4x4 TransformationMatrix = float4x4(In.vRight, In.vUp , In.vLook , In.VTranslation);
   
    float4 vPosition = mul(vector(In.vPosition, 1.f), TransformationMatrix);
    vPosition = mul(vPosition,g_matWorld);
    vPosition = mul(vPosition, g_matView);
    vPosition = mul(vPosition, g_matProj);
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    Out.vLifeTime = In.vLifeTime;
    
    return Out;
}

PS_OUT PS_MAIN(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);
        
    return Out;
}

PS_OUT PS_SNOW(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);
     
    if (Out.vDiffuse.a <= 0.5f)
        discard;
    
    Out.vDiffuse.a = saturate(In.vLifeTime.y - In.vLifeTime.x);
    
    Out.vDiffuse.rgb += (1.f - Out.vDiffuse.a);
    
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
        PixelShader = compile ps_5_0 PS_MAIN();
    } 

    pass Snow
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SNOW();
    }
}