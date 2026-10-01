#include "../../Shader/Default.hlsl"

#define Sampler_WRAP        0
#define Sampler_CLAMP       1
#define Sampler_MIRROR      2
#define Sampler_MIRROR_ONCE 3
#define Sampler_BORDER      4
#define Sampler_TYPE_COUNT  5

float  fLifeTime = 0.f;
float2 vUVSpeed = { 1.f, 1.f };
float2 vUVOffset = { 0.f, 0.f };

int eSamplerType = Sampler_WRAP;

float4 Sampler_Texture_Anim(int iTextureIndex, float2 vTexcoord)
{
    float2 vAnimUV = vUVOffset + vTexcoord;
    
    switch (eSamplerType)
    {
        case Sampler_WRAP:          return g_Textures[iTextureIndex].Sample(Wrap_Sampler, vAnimUV);
        case Sampler_CLAMP:         return g_Textures[iTextureIndex].Sample(ClampSampler, vAnimUV);
        case Sampler_MIRROR:        return g_Textures[iTextureIndex].Sample(MirrorSampler, vAnimUV);
        case Sampler_MIRROR_ONCE:   return g_Textures[iTextureIndex].Sample(MirrorOnceSampler, vAnimUV);
        case Sampler_BORDER:        return g_Textures[iTextureIndex].Sample(BorderSampler, vAnimUV);
        default:                    return float4(1.f, 1.f, 1.f, 1.f);
    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}


float4 Sampler_Texture_NonAnim(int iTextureIndex, float2 vTexcoord)
{
    switch (eSamplerType)
    {
        case Sampler_WRAP:          return g_Textures[iTextureIndex].Sample(Wrap_Sampler, vTexcoord);
        case Sampler_CLAMP:         return g_Textures[iTextureIndex].Sample(ClampSampler, vTexcoord);
        case Sampler_MIRROR:        return g_Textures[iTextureIndex].Sample(MirrorSampler, vTexcoord);
        case Sampler_MIRROR_ONCE:   return g_Textures[iTextureIndex].Sample(MirrorOnceSampler, vTexcoord);
        case Sampler_BORDER:        return g_Textures[iTextureIndex].Sample(BorderSampler, vTexcoord);
        default:                    return float4(1.f, 1.f, 1.f, 1.f);
    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}

float4 Sampler_Material_Anim(int iMaterialIndex, float2 vTexcoord)
{
    float2 vAnimUV = vUVOffset + vTexcoord;
    
    switch (eSamplerType)
    {
        case Sampler_WRAP:           return g_MaterialTexture[iMaterialIndex].Sample(Wrap_Sampler, vAnimUV);
        case Sampler_CLAMP:          return g_Textures[iMaterialIndex].Sample(ClampSampler, vAnimUV);
        case Sampler_MIRROR:         return g_Textures[iMaterialIndex].Sample(MirrorSampler, vAnimUV);
        case Sampler_MIRROR_ONCE:    return g_Textures[iMaterialIndex].Sample(MirrorOnceSampler, vAnimUV);
        case Sampler_BORDER:         return g_Textures[iMaterialIndex].Sample(BorderSampler, vAnimUV);
        default:                     return float4(1.f, 1.f, 1.f, 1.f);
    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}


float4 Sampler_Material_NonAnim(int iMaterialIndex, float2 vTexcoord)
{
    switch (eSamplerType)
    {
        case Sampler_WRAP:          return g_Textures[iMaterialIndex].Sample(Wrap_Sampler, vTexcoord);
        case Sampler_CLAMP:         return g_Textures[iMaterialIndex].Sample(ClampSampler, vTexcoord);
        case Sampler_MIRROR:        return g_Textures[iMaterialIndex].Sample(MirrorSampler, vTexcoord);
        case Sampler_MIRROR_ONCE:   return g_Textures[iMaterialIndex].Sample(MirrorOnceSampler, vTexcoord);
        case Sampler_BORDER:        return g_Textures[iMaterialIndex].Sample(BorderSampler, vTexcoord);
        default:                    return float4(1.f, 1.f, 1.f, 1.f);
    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}




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
    
    float2 vAnimUV = vUVOffset + In.vTexcoord;
    
    float4 vTexture = g_Textures[0].Sample(DefaultSampler, vAnimUV );
    
    Out.vDiffuse = vTexture;
   
    
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


PS_OUT PS_SWORDSLASH(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse *= g_Color * g_Color.a;
    
    if (Out.vDiffuse.a < 0.1f)
        discard;
  
    
    
        return Out;
}

PS_OUT PS_DEAD_SMOKE(VS_OUT In)
{
    PS_OUT Out;
    
    float4 vTexture = g_Textures[0].Sample(DefaultSampler, In.vTexcoord);
 
    Out.vDiffuse.rgb = g_Color.rgb;
    Out.vDiffuse.a = g_Color.a * vTexture.g;
    
    return Out;
} 




technique11 DefaultTechnique
{
    pass Default
    {
        SetRasterizerState(RS_NONCULL);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader  = compile  ps_5_0 PS_MAIN();
    } 

    pass SwordSlash
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORDSLASH();
    }


    pass Dead_Smoke
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DEAD_SMOKE();
    }

}