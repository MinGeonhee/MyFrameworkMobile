#include "../../Shader/Default.hlsl"  


float4 g_CellColorVector = { 0.f, 0.f, 0.f, 1.f };


struct VS_IN
{
    float3 vPosition : Position;
    float4 vColor    : COLOR;
};


struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vColor    : COLOR;
    float4 vWorldPos : TEXCOORD;
};


struct PS_OUT
{
    float4 vDiffuse : SV_Target0;
};


RasterizerState DefaultRasterizer
{
    CullMode = NONE;
    FillMode = WIREFRAME;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    In.vPosition.y += 0.1f;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_matWorld);
    
    Out.vWorldPos = Out.vPosition;
    
    Out.vPosition = mul(Out.vPosition, g_matViewProj);
    
    Out.vColor = In.vColor;
    
    return Out;
}

VS_OUT VS_INCELL(VS_IN In)
{
    VS_OUT Out;
    
    In.vPosition.y += 0.2f;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_matWorld);
    
    Out.vWorldPos = Out.vPosition;
    
    Out.vPosition = mul(Out.vPosition, g_matViewProj);
    
    Out.vColor = In.vColor;
    
    return Out;
}


RasterizerState CellRasterizer
{
    CullMode = NONE;
    FillMode = WIREFRAME;
};

RasterizerState CellSelectedRasterizer
{
    CullMode = NONE;
    FillMode = SOLID;
};



PS_OUT PS_MAIN(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = In.vColor;
 

    return Out;
}

PS_OUT PS_INCELL(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_CellColorVector;
    
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

    pass InCell
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_INCELL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_INCELL();
    }
}

