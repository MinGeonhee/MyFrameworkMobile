#include "../../Shader/Default.hlsl"



row_major float4x4 g_GlobalInvProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_ShadeTexture;
Texture2D g_DepthTexture;
Texture2D g_SpecularTexture;


float4 g_vMtrlAmbient = float4(1.f, 1.f, 1.f, 1.f);
float4 g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f);



RasterizerState RS_SCISSOR_VIEWPORTRECT
{
    FillMode = Solid;
    CullMode = None;
    ScissorEnable = true;
};


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

struct PS_IN
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vColor : SV_Target0;
};

struct PS_OUT_LIGHT 
{
    float4 vShade       : SV_Target0;
    float4 vSpecular    : SV_Target1;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    vector vPosition = mul(vector(In.vPosition, 1.f), g_matWorld);
    vPosition = mul(vPosition, g_matCanvas );
    
    
    
    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

 
PS_OUT_LIGHT PS_DIRECTIONAL_LIGHT(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    float4 vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vDepth  = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);

    /* A-plan : fCamIndex >= 0 means index reject is on. Skip pixels owned by others. */
    if (g_tCameraInfo.fCamIndex >= 0.f && abs(vDepth.z - g_tCameraInfo.fCamIndex) > 0.5f)
        discard;
    vNormal = float4(vNormal.xyz * 2.f - 1.f,0.f);
    
    
    float4 vShade = Get_Shade(tGlobalLightBuffer.vLightDir, vNormal) + (tGlobalLightBuffer.vLightAmbient * g_vMtrlAmbient);
    
    Out.vShade = tGlobalLightBuffer.vLightDiffuse * saturate(vShade);
    
    
    float fPixelX = In.vTexcoord.x * 1280.0f;
    float fPixelY = In.vTexcoord.y * 720.0f;
    
    float fCorrectedNDC_X = (2.0f * (fPixelX - g_tCameraInfo.fStartX) / g_tCameraInfo.fWidth) - 1.0f;
    float fCorrectedNDC_Y = 1.0f - (2.0f * (fPixelY - g_tCameraInfo.fStartY) / g_tCameraInfo.fHeight);
   
    float4 vNDC = float4(fCorrectedNDC_X, fCorrectedNDC_Y, vDepth.x, 1.0f);
    float4 vViewPos = mul(vNDC, g_matProjInv);
    vViewPos /= vViewPos.w;
    float4 vWorldPos = mul(vViewPos, g_matViewInv);
    
    
    float4 vReflect = reflect(normalize(tGlobalLightBuffer.vLightDir), vNormal);
    
    float4 vLook = vWorldPos - g_CamPosition;
    
    Out.vSpecular = (tGlobalLightBuffer.vLightSpecular * g_vMtrlSpecular) * (pow(max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f),50.f));
    
    
    return Out;
}

PS_OUT_LIGHT PS_POINT_LIGHT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vDepth = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);

    /* A-plan : fCamIndex >= 0 means index reject is on. Skip pixels owned by others. */
    if (g_tCameraInfo.fCamIndex >= 0.f && abs(vDepth.z - g_tCameraInfo.fCamIndex) > 0.5f)
        discard;
    
    vNormal = float4(vNormal.xyz * 2.f - 1.f, 0.f);
    
        
    float fPixelX = In.vTexcoord.x * 1280.0f;
    float fPixelY = In.vTexcoord.y * 720.0f;
    
    float fCorrectedNDC_X = (2.0f * (fPixelX - g_tCameraInfo.fStartX) / g_tCameraInfo.fWidth) - 1.0f;
    float fCorrectedNDC_Y = 1.0f - (2.0f * (fPixelY - g_tCameraInfo.fStartY) / g_tCameraInfo.fHeight);
   
    float4 vNDC = float4(fCorrectedNDC_X, fCorrectedNDC_Y, vDepth.x, 1.0f);
    float4 vViewPos = mul(vNDC, g_matProjInv);
    vViewPos /= vViewPos.w;
    float4 vWorldPos = mul(vViewPos, g_matViewInv);

    
    int PointNum = iPointLightNum;
    /* measurement : repeats the light loop so the GPU, not CPU submission, is the
       bottleneck. (i + rep) % PointNum sums the same light set each pass, so the
       image is unchanged after the divide below. Remove with the bench code. */
    static const int MEASURE_REP = 8;
    [loop] for (int rep = 0; rep < MEASURE_REP; ++rep)
    {
    
       
    for (int i = 0; i < PointNum; ++i)
    {
        float4 vLightDir = vWorldPos - arrPointLightBuffer[(i + rep) % PointNum].vPosition;
        
        float fDistance = length(vLightDir);
        
        float Ratio = saturate((arrPointLightBuffer[(i + rep) % PointNum].fRange - fDistance) / arrPointLightBuffer[(i + rep) % PointNum].fRange);
        
        float4 vShade = max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + (g_vMtrlAmbient * arrPointLightBuffer[(i + rep) % PointNum].vLightAmbient);
        float4 vFinalShad = arrPointLightBuffer[(i + rep) % PointNum].vLightDiffuse * saturate(vShade) * Ratio;
        
        Out.vShade += vFinalShad;
        
        float4 vLook = vWorldPos - g_CamPosition;
        float4 vReflect = reflect(normalize(vLightDir), vNormal);
        float4 vSpecular = (g_vMtrlSpecular * arrPointLightBuffer[(i + rep) % PointNum].vLightSpecular) * (pow(max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f), 50.f)) * Ratio;
        Out.vSpecular += vSpecular;
    }
    }
    Out.vShade /= MEASURE_REP;
    Out.vSpecular /= MEASURE_REP;
    
    return Out;
}



PS_OUT_BACKBUFFER PS_COMBINED_LIGHT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    //if (vDiffuse.a < 0.1f)
    //    discard;
    
    float4 vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    
   // vDiffuse.xyz *= 1.5f;
    
    Out.vColor = vDiffuse * vShade + vSpecular;
    
    return Out;
}


technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader  = compile  ps_5_0 PS_DEBUG();
    }

    pass Directional_Light
    {
        SetRasterizerState(RS_SCISSOR_VIEWPORTRECT);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_AddBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DIRECTIONAL_LIGHT();
    }

    pass Point_Light
    {
        SetRasterizerState(RS_SCISSOR_VIEWPORTRECT);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_AddBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POINT_LIGHT();
    }
   
    // 두 텍스처를 들고와서 Compute해주는 단계 최종 빽버퍼에 그려줄 단계
    pass Combined
    {
        // scissor so PERCAM combines only the current camera's rect
        SetRasterizerState(RS_SCISSOR_VIEWPORTRECT);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_COMBINED_LIGHT();
    }


}