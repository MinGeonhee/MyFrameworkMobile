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


struct PS_OUT
{
    float4 vDiffuse : SV_Target0;
};



VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    vector vPosition = mul(vector(In.vPosition, 1.f), g_matWorld);
    vPosition = mul(vPosition, g_matView);
    vPosition = mul(vPosition, g_matProj);
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}


VS_OUT VS_WORLDUI(VS_IN In)
{
    VS_OUT Out;
    
    vector vPosition = mul(vector(In.vPosition, 1.f), g_matWorld);
    vPosition = mul(vPosition, g_matView);
    vPosition = mul(vPosition, g_matProj);
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}


PS_OUT PS_MAIN(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    if (Out.vDiffuse.a < 0.5f)
        discard;
    
    
    return Out;
}

PS_OUT PS_EFFECT(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse *= g_Color;
    
    if(Out.vDiffuse.a < 0.05f)
        discard;
    
    return Out;
}


PS_OUT PS_WORLDUI(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    if (Out.vDiffuse.a < 0.5f)
        discard;
    
    
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

    pass WorldUI
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_WORLDUI();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_WORLDUI();
    }

    pass Effect
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AddBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EFFECT();
    }

}