#include "../../Shader/Default.hlsl"  

//이게 카매라 기준으로 앞쪽 뒤쪽
#define CornerCount 8
#define FRONT_LT 0
#define FRONT_RT 1 
#define FRONT_RB 2
#define FRONT_LB 3

// 쉐이더 파일을 3개로 나눌려고 했던 이유가뭘까 아마 원? Sphere때문이긴햇어...
// 그렇다면 어떤식으로 하는게 제일 베스트일까?
// 반대편은 음 거구로뭔갈해야할지 고민좀해보자
#define BACK_LT  0
#define BACK_RT  1 
#define BACK_RB  2
#define BACK_LB  3

#define Sphere   0
#define Box      1   
#define RotBox   2

int g_ColliderType = { 0.f };

cbuffer g_ColliderBufferBox
{
    float4 tBoxCenter  = {1.f,1.f,1.f,1.f};     
    float4 tBoxExtents = { 1.f, 1.f, 1.f, 0.f };
    float4 tBoxColor   = { 1.f, 0.f, 0.f, 0.f };
};

cbuffer g_ColliderBufferRotBox
{
    float4x4 matColliderRotBoxWorld={1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f};
    float4 tColor = {1.f,0.f,0.f,1.f};
};


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
    float4 vColor : SV_Target;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
     
    if (g_ColliderType == Sphere)
    {
        
    }
    else if( g_ColliderType == Box)
    {
        Out.vPosition = float4(In.vPosition,1.f) * tBoxExtents + tBoxCenter; 
        Out.vColor = tBoxColor;
    }
    else if (g_ColliderType == RotBox)
    {
        Out.vPosition = mul(float4(In.vPosition, 1.f), matColliderRotBoxWorld);
        Out.vColor = In.vColor;
    }
    
    Out.vPosition = mul(Out.vPosition, g_matViewProj);
    
    return Out;
}

PS_OUT PS_MAIN(VS_OUT In)
{
    PS_OUT Out;
    
    Out.vColor = In.vColor;
    
    return Out;
}
    
technique11 DefaultTechnique
{

    pass Default
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Collided
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}

