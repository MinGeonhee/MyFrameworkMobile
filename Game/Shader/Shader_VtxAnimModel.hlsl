#include "../../Shader/Default.hlsl"

#define g_iMaxNumBones 512

row_major float4x4 g_arrayBoneMatrix[g_iMaxNumBones];


struct VS_IN
{
    float3 vPosition    : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vBinormal    : BINORMAL;
    float2 vTexcoord    : TEXCOORD0;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vNormal   : NORMAL;
    float4 vTangent  : TANGENT;
    float4 vBinormal : BINORMAL;
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
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos  : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal  : SV_TARGET1;
    float4 vDepth   : SV_TARGET2;
};



VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float fWeightw = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    matrix BoneMatrix = g_arrayBoneMatrix[In.vBlendIndex.x] * In.vBlendWeight.x +
        g_arrayBoneMatrix[In.vBlendIndex.y] * In.vBlendWeight.y +
        g_arrayBoneMatrix[In.vBlendIndex.z] * In.vBlendWeight.z +
        g_arrayBoneMatrix[In.vBlendIndex.w] * fWeightw;
   
    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);

    
    matrix matWV, matWVP;
    matWV = mul(g_matWorld, g_matView);
    matWVP = mul(matWV,g_matProj);
    
    Out.vPosition = mul(vPosition , matWVP);
    Out.vNormal = normalize(mul(vNormal, g_matWorld));
    Out.vTangent = normalize(mul(mul(float4(In.vTangent, 0.f),BoneMatrix ) , g_matWorld));
    Out.vBinormal = normalize(mul(mul(float4(In.vBinormal, 0.f), BoneMatrix), g_matWorld));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vPosition , g_matWorld);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    float4 vNormalMap = g_MaterialTexture[NORMALMAP].Sample(DefaultSampler, In.vTexcoord);
    
    if (Out.vDiffuse.a < 0.2f)
        discard;
    
    float2 vRawXY = vNormalMap.rg * 2.f - 1.f;
    float4 vNormal = float4(vRawXY, sqrt(1.f - dot(vRawXY, vRawXY)), 0.f);
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    vNormal = float4(mul(vNormal.xyz, WorldMatrix), 0.f);
    
    
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w ,In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    return Out;
}

PS_OUT PS_NONE_DAMAGE(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    if (Out.vDiffuse.a <= 0.1f)
        discard;
   
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    float fBlink = sin((g_DT * 10.f) + 1.f) * 0.5f;
    float Alpha = 0.3f + (fBlink * 0.7f);
    
    Out.vDiffuse.a *= Alpha;
    
    return Out;
}


PS_OUT PS_HIT(PS_IN In)
{

    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
 
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    
    float3 HitColor = Out.vDiffuse.xyz;
    float3 RedTint = { 1.f, 0.1f, 0.1f };
    
    HitColor = lerp(HitColor, RedTint, 0.3f);
    
    Out.vDiffuse.xyz = HitColor;
       
    
    return Out;
}


PS_OUT PS_DEAD(PS_IN In)
{

    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
       
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    Out.vDiffuse.a = 1 - g_DT;
    
    
    return Out;
}

PS_OUT PS_SPAWN(PS_IN In)
{
    PS_OUT Out;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
       
    Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5, 0.f);
    Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_tCameraInfo.fFar, g_tCameraInfo.fCamIndex, 0.f);
    
    // 받아온 진행률에대한 DT값만 넣어주자
    Out.vDiffuse.a = g_DT;
    
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


    pass Hit
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_HIT();
    }


    pass Dead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DEAD();
    }

    pass Spawn
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPAWN();
    }


    pass NoneDamage
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NONE_DAMAGE();
    }

}

