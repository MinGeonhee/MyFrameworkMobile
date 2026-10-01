#include "../../Shader/Default.hlsl"

#define Sampler_WRAP        0
#define Sampler_CLAMP       1
#define Sampler_MIRROR      2
#define Sampler_MIRROR_ONCE 3
#define Sampler_BORDER      4
#define Sampler_TYPE_COUNT  5

#define PI                  3.1415926535f

float   fLifeTime = 0.f;
float2  vUVSpeed = { 1.f, 1.f};
float2  vUVOffset = { 0.f , 0.f};
int     eSamplerType = Sampler_WRAP;


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
        case Sampler_WRAP:          return g_MaterialTexture[iMaterialIndex].Sample(Wrap_Sampler, vAnimUV);
        case Sampler_CLAMP:         return g_MaterialTexture[iMaterialIndex].Sample(ClampSampler, vAnimUV);
        case Sampler_MIRROR:        return g_MaterialTexture[iMaterialIndex].Sample(MirrorSampler, vAnimUV);
        case Sampler_MIRROR_ONCE:   return g_MaterialTexture[iMaterialIndex].Sample(MirrorOnceSampler, vAnimUV);
        case Sampler_BORDER:        return g_MaterialTexture[iMaterialIndex].Sample(BorderSampler, vAnimUV);
        default:                    return float4(1.f, 1.f, 1.f, 1.f);

    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}


float4 Sampler_Material_NonAnim(int iMaterialIndex, float2 vTexcoord)
{
    switch (eSamplerType)
    {
        case Sampler_WRAP:          return g_MaterialTexture[iMaterialIndex].Sample(Wrap_Sampler, vTexcoord);
        case Sampler_CLAMP:         return g_MaterialTexture[iMaterialIndex].Sample(ClampSampler, vTexcoord);
        case Sampler_MIRROR:        return g_MaterialTexture[iMaterialIndex].Sample(MirrorSampler, vTexcoord);
        case Sampler_MIRROR_ONCE:   return g_MaterialTexture[iMaterialIndex].Sample(MirrorOnceSampler, vTexcoord);
        case Sampler_BORDER:        return g_MaterialTexture[iMaterialIndex].Sample(BorderSampler, vTexcoord);
        default:                    return float4(1.f, 1.f, 1.f, 1.f);
    }
    
    return float4(1.f, 1.f, 1.f, 1.f);
}



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
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0; 
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos  : TEXCOORD2;
};

// VS_OUT /  PS_IN 구조체는 동일해야한다./

struct PS_IN
{
    float4 vPosition : SV_Position;
    float4 vNormal :   NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
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

struct PS_BACKBUFFER_OUT
{
    float4 vDiffuse : SV_Target0;
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


PS_BACKBUFFER_OUT PS_MAIN(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    float4 vTexture = Sampler_Texture_Anim(0, In.vTexcoord);
    
    Out.vDiffuse.rgb = g_Color.rgb;
    
    Out.vDiffuse.a = vTexture.a; 
    
    if(Out.vDiffuse.a < 0.3f)
        discard;
    
    
    return Out;
}


PS_BACKBUFFER_OUT PS_TEX_CA_MIX_MY_CA(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    float4 vTexture = Sampler_Texture_Anim(0,In.vTexcoord);
       
    float fFinalAlpha = vTexture.r * g_Color.a;
   
    Out.vDiffuse.rgb = vTexture.rgb * g_Color.rgb;
    Out.vDiffuse.a = fFinalAlpha;
    
    
    if (Out.vDiffuse.a < 0.4f)
        discard;
    
    return Out;
}



PS_BACKBUFFER_OUT PS_SWORDSLASH(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    float2 vRotatedUV = In.vTexcoord.yx;
    Out.vDiffuse = g_MaterialTexture[DIFFUSE].Sample(Wrap_Sampler, In.vTexcoord.yx);
    float4 vNormalMap = g_MaterialTexture[NORMALMAP].Sample(Wrap_Sampler, In.vTexcoord);
    
    float2 vRawXY = vNormalMap.rg * 2.f - 1.f;
    float4 vNormal = float4(vRawXY, sqrt(1.f - dot(vRawXY, vRawXY)), 0.f);
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    vNormal = float4(mul(vNormal.xyz, WorldMatrix), 0.f);
  
    
    float fProgressRation = g_Ratio * vUVSpeed.y;
    float fProgressMask = saturate(1.0f - abs(In.vTexcoord.y - fProgressRation) * 2.0f);
    float fLifeAplah = 4.0f * g_Ratio * (1.0f - g_Ratio);
    
   
    Out.vDiffuse.a = g_Color.a * fLifeAplah * fProgressMask;
    
    Out.vDiffuse.xyz = g_Color.rgb;
    
    return Out;
}


PS_BACKBUFFER_OUT PS_MONSTERHIT(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    float4 vTexture = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);    
    //float fDist = abs(0.5f - In.vTexcoord) *2.f -1.f ;
    
    float Alpha = 1 - g_Ratio;
    
    
    Out.vDiffuse = g_Color;
    Out.vDiffuse.a = Alpha * g_Color.a;
    
    return Out;
}


PS_BACKBUFFER_OUT PS_SWORDCHARGING_ATTACK_SWORD(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    float4 vTexture = g_MaterialTexture[DIFFUSE].Sample(DefaultSampler, In.vTexcoord);    
    
    //float fDist = abs(0.5f - In.vTexcoord) *2.f -1.f ;
    
    float fAlpha = sin(g_Ratio * PI );
        
    float fFillProgress = saturate(fAlpha * 2.f);
    
    if (In.vTexcoord.x < 1 - fFillProgress )
        discard;
    
    
    
    
    Out.vDiffuse = g_Color;
    Out.vDiffuse.a = fAlpha * g_Color.a;
    
    return Out;
}

PS_BACKBUFFER_OUT PS_SWORD_AURA(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    float4 vTexture = Sampler_Texture_Anim(0, In.vTexcoord);
    
    Out.vDiffuse.rgb = g_Color.rgb;
    
    Out.vDiffuse.a = vTexture.r * g_Color.a;
    
    return Out;
}


PS_BACKBUFFER_OUT PS_SWORD_CHARGING_ST_SWORD_AURA(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    float4 vTexture = Sampler_Texture_Anim( 0  , In.vTexcoord);
       
    float fRatioAlpha = g_DT / 1.5f;
    
    if (In.vTexcoord.y < (1.f - fRatioAlpha) )
        discard; 
    
    Out.vDiffuse = vTexture * g_Color;
    
    Out.vDiffuse.a =  vTexture.r *  g_Color.a;
   
    
    return Out;
}

PS_BACKBUFFER_OUT PS_SWORD_CHARGING_On(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
   
    float4 vTexture = Sampler_Texture_Anim(0, In.vTexcoord);
    
    Out.vDiffuse    =    vTexture * g_Color;
    Out.vDiffuse.a  =    vTexture.r *  g_Color.a;
    
    
    return Out;
}


PS_BACKBUFFER_OUT PS_SHIELD_HIT_RING(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vGrad_Texture = Sampler_Texture_Anim(0,In.vTexcoord);
    float4 vMask_Texture = Sampler_Texture_Anim(1,In.vTexcoord);
    
    if(vMask_Texture.r < 0.4f)
        discard;
    
    float4 vFinalColor = vGrad_Texture * vMask_Texture * g_Color;
    
    Out.vDiffuse = vFinalColor;
    
    return Out;
}




PS_BACKBUFFER_OUT PS_SHIELD_HIT_FLASH(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
       
    Out.vDiffuse = g_Color;
  
    Out.vDiffuse.a = In.vTexcoord.y;
    
    Out.vDiffuse.a *= pow(sin(g_Ratio * PI), 2.f);
   
    return Out;
}



PS_BACKBUFFER_OUT PS_MAGIC_CHARGING(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vGrad_Texture = Sampler_Texture_Anim(0, In.vTexcoord);
    float4 vMask_Texture = Sampler_Texture_Anim(1, In.vTexcoord);
   
    
    if (vGrad_Texture.r < 0.4f)
        discard;
    
    if (vMask_Texture.r < 0.4f)
        discard;
    
    float4 vFinalColor = vGrad_Texture * vMask_Texture * g_Color;
    
    if (vFinalColor.a < 0.3f)
        discard;
    
    Out.vDiffuse = vFinalColor;
    
    return Out;
}


PS_BACKBUFFER_OUT PS_SPECIALMAGIC_CHARGING_THUNDER(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vGrad_Texture = Sampler_Texture_Anim(0, In.vTexcoord);
    float4 vMask_Texture = Sampler_Texture_Anim(1, In.vTexcoord);
   
    
    float4 vFinalColor = vGrad_Texture * vMask_Texture * g_Color;
   
    Out.vDiffuse = vFinalColor;
    
    return Out;
}


PS_BACKBUFFER_OUT PS_SPECIALMAGIC_BULLET(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vGrad_Texture = Sampler_Texture_Anim(0, In.vTexcoord);
    float4 vMask_Texture = Sampler_Texture_NonAnim(1, In.vTexcoord);
    float4 vGlow_Texture = Sampler_Texture_Anim(2 , In.vTexcoord);
    
    float4 vFinalColor = vGrad_Texture * vMask_Texture * g_Color;
   
    
    float fPlusColor = vGlow_Texture.r;
    
    fPlusColor *= (1 - In.vTexcoord.y);
    
    fPlusColor = pow(fPlusColor, 5.f);
        
        
    vFinalColor.rgb += fPlusColor;
    
 
    
    Out.vDiffuse = vFinalColor;
    
    return Out;
}

PS_BACKBUFFER_OUT PS_DEAD_SMOKE(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vTexture = Sampler_Texture_Anim(0, In.vTexcoord);
    
    Out.vDiffuse.rgb = g_Color.rgb;
    Out.vDiffuse.a = g_Color.a * vTexture.g;
    
    
    return Out;
}


PS_BACKBUFFER_OUT PS_OBJECT_DEAD(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vDiffuseTexture = Sampler_Texture_NonAnim(0, In.vTexcoord);
    float4 vNormalTexture  = Sampler_Texture_NonAnim(1, In.vTexcoord);
    
    float fAplha = 1 - g_Ratio;
    
    
    Out.vDiffuse = vDiffuseTexture;
   
    Out.vDiffuse.a = fAplha ;
    
    if(Out.vDiffuse.a < 0.1f)
        discard;
   
    
    return Out;
}

PS_BACKBUFFER_OUT PS_SMOKE_LINE(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;

    float4 vDistortionTexture   = Sampler_Texture_Anim(0, In.vTexcoord);
    // UV 비틀기 
    float2 distoredUV = In.vTexcoord + (vDistortionTexture.xy * 0.5);
    
    float2 DissloveUV;
    DissloveUV.y = vUVOffset.y;
    
    
    float4 vDissloveTexture = Sampler_Texture_Anim(1, distoredUV);
    float4 vMaskTexture     = Sampler_Texture_NonAnim(2, distoredUV);

    
    float fDissolveAlpha = step(0.1, vDissloveTexture.r);
    
    Out.vDiffuse = fDissolveAlpha * vMaskTexture.a * g_Color.a;
    
    if (Out.vDiffuse.a < 0.1f)
        discard;
   
    Out.vDiffuse.rgb *= g_Color.rgb;
    
    
    return Out;
}


PS_BACKBUFFER_OUT PS_LAVASPLASH_BULLET(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    /* Noise 텍스처  */ 
    float4 vMaskTexture_1 = Sampler_Texture_NonAnim(0, In.vTexcoord);
    float4 vMaskTexture_2 = Sampler_Texture_Anim(1, In.vTexcoord);
    float4 vGlowTexture = Sampler_Texture_Anim(2, In.vTexcoord);
    
    Out.vDiffuse = vMaskTexture_1 * vMaskTexture_2 * g_Color;
    
    
    if(Out.vDiffuse.a < 0.3f)
        discard;
    
    
    return Out;
}


PS_BACKBUFFER_OUT PS_FIRE(PS_IN In)
{
    PS_BACKBUFFER_OUT Out;
    
    /* Noise 텍스처  */ 
    float4 vDisolveTexture = Sampler_Texture_Anim(0, In.vTexcoord);
    float4 vMaskTexture    = Sampler_Texture_NonAnim(1, In.vTexcoord);
    
    
    Out.vDiffuse = vDisolveTexture * vMaskTexture * g_Color;
    
    
    if (Out.vDiffuse.a < 0.7f)
        discard;
    
    Out.vDiffuse.rgb *= 2.0f;
    
    
    return Out;
}



technique11 DefaultTechnique
{

    pass Default
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    
    pass Tex_CA_Mix_My_CA
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TEX_CA_MIX_MY_CA();
    }

    pass SwordSlash
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORDSLASH();
    }


    pass SwordCharging_Attack_Sword
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORDCHARGING_ATTACK_SWORD();
    }

    pass MonsterHit
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MONSTERHIT();
    }

    pass Sword_Aura
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORD_AURA();
    }

    pass Sword_Charging_ST_Sword_Aura
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORD_CHARGING_ST_SWORD_AURA();
    }

    pass Sword_Charging_On
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SWORD_CHARGING_On();
    }


    pass Shield_Hit_Ring
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHIELD_HIT_RING();
    }

    pass Shield_Hit_Flash
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHIELD_HIT_FLASH();
    }

    pass Magic_Charging
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAGIC_CHARGING();
    }

    pass SpecialMagic_Charging_Thunder
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPECIALMAGIC_CHARGING_THUNDER();
    }

    pass SpecialMagic_Bullet
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPECIALMAGIC_BULLET();
    }


    pass Object_Dead
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OBJECT_DEAD();
    }

    pass Smoke_Line
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SMOKE_LINE();
    }

    pass LavaSplash_Bullet
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LAVASPLASH_BULLET();
    }

    pass Fire
    {
        SetRasterizerState(RS_NONCULL);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FIRE();
    }


}

