#include "../../Shader/Default.hlsl"

float2  g_vTextureGrid = {1.f,1.f};
int     g_iTotalFrameCount = {1};

struct VS_IN
{
    float3              vPosition            : POSITION;
    row_major float4x4  TransformationMatrix : WORLD;
    float2              vLifeTime            : TEXCOORD0;
    uint                iTextureIndex        : TEXCOORD1;
};

struct VS_OUT
{
    float4 vPosition     : Position;
    float2 vPSize        : PSIZE;
    float2 vLifeTime     : TEXCOORD0;
    uint   iTextureIndex : TEXCOORD1;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
   
    Out.vPosition = mul(float4(In.vPosition,1.f), In.TransformationMatrix);
    Out.vPosition = mul(Out.vPosition,g_matWorld);
    Out.vPSize = float2(length(In.TransformationMatrix._11_12_13), length(In.TransformationMatrix._21_22_23));
    Out.vLifeTime = In.vLifeTime;
    Out.iTextureIndex = In.iTextureIndex;
    
    return Out;
}


struct GS_IN
{
    float4 vPosition     : POSITION;
    float2 vPSize        : PSIZE;
    float2 vLifeTime     : TEXCOORD0;
    uint   iTextureIndex : TEXCOORD1;
};


struct GS_OUT
{
    float4 vPosition     : SV_POSITION;
    float2 vTexcoord     : TEXCOORD0;
    float2 vLifeTime     : TEXCOORD1;
    uint   iTextureIndex : TEXCOORD2;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> OutStream)
{
    GS_OUT Out[4];
    
    float3 vLook = (g_CamPosition - In[0].vPosition).xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * In[0].vPSize.x;
    float3 vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y;
    
    matrix matVP = mul(g_matView, g_matProj);
    
    Out[0].vPosition = mul(vector(In[0].vPosition.xyz + vRight + vUp, 1.f), matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].iTextureIndex= In[0].iTextureIndex;
    
    Out[1].vPosition = mul(vector(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].iTextureIndex = In[0].iTextureIndex;


    Out[2].vPosition = mul(vector(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].iTextureIndex = In[0].iTextureIndex;

  
    Out[3].vPosition = mul(vector(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].iTextureIndex= In[0].iTextureIndex;

    
    OutStream.Append(Out[0]);
    OutStream.Append(Out[1]);
    OutStream.Append(Out[2]);
    OutStream.RestartStrip();
    
    OutStream.Append(Out[0]);
    OutStream.Append(Out[2]);
    OutStream.Append(Out[3]);
    OutStream.RestartStrip();
}


struct PS_IN
{
    float4 vPosition     : SV_POSITION;
    float2 vTexcoord     : TEXCOORD0;
    float2 vLifeTime     : TEXCOORD1;
    uint   iTextureIndex : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
};


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    Out.vDiffuse = g_Texture.Sample(DefaultSampler, In.vTexcoord);
        
    if (Out.vDiffuse.a <= 0.3f)
        discard;
    
    Out.vDiffuse.a = saturate(In.vLifeTime.y - In.vLifeTime.x);
    
    return Out;
}


PS_OUT PS_SMOKE(PS_IN In)
{
    PS_OUT Out;
    
    float2 vGird = g_vTextureGrid;
    float fFrameIndex = (float)In.iTextureIndex;
    
    float fCol = fmod(fFrameIndex, vGird.y);
    float fRow = floor(fFrameIndex / vGird.x);
    
    float2 vNewUV = float2((In.vTexcoord.x + fCol) / vGird.x, (In.vTexcoord.y + fRow) / vGird.y);
 
    
    float4 vTexture = g_Texture.Sample(DefaultSampler, vNewUV );
    
    Out.vDiffuse.rgb = g_Color.rgb;
    Out.vDiffuse.a   = vTexture.g * g_Color.a;
    
    float fRatio = fFrameIndex / (float)g_iTotalFrameCount;
    
    Out.vDiffuse.a *= (1 - fRatio);
    
    if(Out.vDiffuse.a < 0.05f)
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
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    } 

    pass Smoke
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NONE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile   vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile    ps_5_0  PS_SMOKE();
    }
}