#include "../../Shader/Default.hlsl"



struct VS_IN
{
    float3 vPosition    : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vBinormal    : BINORMAL;
    float2 vTexcoord    : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vNormal   : NORMAL;
    float4 vTangent  : TANGENT;
    float4 vBinormal : vBinormal;
    float2 vTexcoord : TEXCOORD0; 
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos  : TEXCOORD2;
};

// VS_OUT /  PS_IN 구조체는 동일해야한다./

struct PS_IN
{
    float4 vPosition : SV_Position;
    float4 vNormal   : NORMAL;
    float4 vTangent  : TANGENT;
    float4 vBinormal : vBinormal;
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
    Out.vNormal     = normalize(mul(vector(In.vNormal, 0.f), g_matWorld));
    Out.vTangent    = normalize(mul(float4(In.vTangent, 0.f), g_matWorld));
    Out.vBinormal   = normalize(mul(float4(In.vBinormal, 0.f), g_matWorld));
    Out.vWorldPos   = mul(vector(In.vPosition , 1.f) , g_matWorld);
    Out.vProjPos    = Out.vPosition;

    
    return Out;
}

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    

    float3 vTangent = normalize(In.vTangent); // 보간되면서 짧아진 놈 다시 1로 늘리기
    float3 vBinormal = normalize(In.vBinormal);
    float3 vNormalGeo = normalize(In.vNormal);
    
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    float4 vNormalMap = g_MaterialTexture[NORMALMAP].Sample(DefaultSampler, In.vTexcoord);        
    if (Out.vDiffuse.a < 0.1)
        discard;
    
    
    float2 vRawXY = vNormalMap.rg * 2.f - 1.f;
    float4 vNormal = float4(vRawXY, sqrt(1.f - dot(vRawXY, vRawXY)), 0.f);
    float3x3 WorldMatrix = float3x3(vTangent, vBinormal * -1.f, vNormalGeo.xyz);
    vNormal = float4(mul(vNormal.xyz, WorldMatrix), 0.f);
  
    
    Out.vNormal = float4(vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w , In.vProjPos.w / g_tCameraInfo.fFar , g_tCameraInfo.fCamIndex, 0.f);
    
    return Out;
}

PS_OUT PS_FIELD(PS_IN In)
{
    PS_OUT Out;
    
    
    float3 vTangent = normalize(In.vTangent); // 보간되면서 짧아진 놈 다시 1로 늘리기
    float3 vBinormal = normalize(In.vBinormal);
    float3 vNormalGeo = normalize(In.vNormal);
    
    Out.vDiffuse        = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);   
    float4 vNormalMap   = g_MaterialTexture[NORMALMAP].Sample(DefaultSampler, In.vTexcoord);
    
    if (Out.vDiffuse.a < 0.25)
        discard;
    
    float2 vRawXY   = vNormalMap.rg * 2.f - 1.f;
    
    float4 vNormal = float4(vRawXY, sqrt(1.f - dot(vRawXY, vRawXY)),1.f);
   
    float3x3 WorldMatrix = float3x3(vTangent, vBinormal * -1.f, vNormalGeo);
    
    vNormal = float4(mul(vNormal.xyz, WorldMatrix),0.f);
    
    Out.vNormal = float4(vNormal.xyz * 0.5f + 0.5, 0.f );
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    return Out;
}

PS_OUT PS_FADEOUT(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    if (Out.vDiffuse.a < 0.05)
        discard;
    
    Out.vDiffuse.a = 1- g_DT;
    
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 1.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    /* W 나누기한 값 저장과 w나누기를위해사용한 w값을 저장하자... */

    
    return Out;
}


PS_OUT PS_EFFECT(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    if (Out.vDiffuse.a < 0.05)
        discard;
    
    Out.vDiffuse  *=  g_Color * g_Color.a;
    
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    /* W 나누기한 값 저장과 w나누기를위해사용한 w값을 저장하자... */

    
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


    pass FadeIn
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FADEOUT();
    }

    pass FadeOut
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FADEOUT();
    }


    pass Field
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FIELD();
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

