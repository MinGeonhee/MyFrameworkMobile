#pragma once


#ifndef			MSG_BOX
#define			MSG_BOX(_message)			MessageBox(nullptr, TEXT(_message), L"System Message", MB_OK)
#endif

#define			NONINDEX					4294967295
#define         ANDROMEDA					-99999999

#define			Lerp(a, b, t)				a + (b - a) * t

#define			g_iMaxNumBones				512
#define         RootNode					-1
#define			No_Neighbor					-1
#define			No_EventIndex				-1

#define			FIELD_CELL_INDEX			-1

#define			NS_BEGIN(NAMESPACE)			namespace NAMESPACE {
#define			NS_END						}

#define			USING(NAMESPACE)			using namespace NAMESPACE;

#define			PURE_VIRTUAL 0
#define			GLOBAL		156

#define			NONE_USE_TEXTURE_ANIM 1
#define			InitMemory(Info) { ZeroMemory(&Info,sizeof(Info));}

#define			Cast_Int(n)		static_cast<int>(n)
#define			Cast_float(n)	static_cast<float>(n)
#define			Cast_Uint(n)	static_cast<unsigned int>(n)

#define			NULLPTR_RETURN_NULL(Ptr)	if( Ptr == nullptr ) { return ; }
#define			NULLPTR_RETURN_FLOAT(Ptr)	if( Ptr == nullptr ) { return 0.f; }
#define			NULLPTR_RETURN_INT(Ptr)		if( Ptr == nullptr ) { return 0; }
#define			NULLPTR_RETURN_NULLPTR(Ptr) if( Ptr == nullptr ) { return nullptr; }
#define			NULLPTR_RETURN_EFAIL(Ptr)	if( Ptr == nullptr ) { return E_FAIL; }

#define			IS_NULLPTR(pInstance)		if ( pInstance == nullptr)
#define			ISNOT_NULLPTR(pInstance)	if (pInstance != nullptr)
#define			NULLCHECK(pInstance)		if ( pInstance == nullptr ) { return true; } else { return false;}

#define			EPSILON						0.001f		

#define			TO_RAD  (XM_PI / 180.f)
#define			To_DEGREE ( 180.f / XM_PI )

#if defined(_WIN32) && !defined(D3DCOLOR_DEFINED)
typedef unsigned long D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))


#define	        SUB_FLAG(Flag_Value,SubFlag)  {if(Flag_Value & SubFlag) Flag_Value ^= SubFlag;}
#define			ADD_FLAG(Flag_Value,AddFlag)  {Flag_Value |= AddFlag;}	
#define			KEEP_ONLY_FLAG(Flag_Value,KeepOnlyFlag) {Flag_Value &= KeepOnlyFlag;}

#define			LT 0
#define			RT 1
#define			RB 2
#define			LB 3

#define			NULLPTR nullptr
#define			CLASS   class
#define			FD		class


#define			Cast_AddCom(pCom)		reinterpret_cast<CComponent**>(&pCom)

#pragma region Shader
/* Shader Register 번호 */

#define			MAX_LIGHTS					50

#define			Shader_TransfromBuffer		0
#define			Shader_CameraBuffer			1


#define			Shader_CameraBuffer_MatView		0
#define			Shader_CameraBuffer_MatProj			1
#define			Shader_CameraBuffer_MatViewProj	2

#define			Shader_Texture							0
#define			Shader_DiffuseTexture					1
#define			Shader_MaskTexture						2


#define			GB_Shader_Diffuse     0
#define			GB_Shader_Ambient     1
#define			GB_Shader_Normal      2
#define			GB_Shader_MtlSpecular 3
#define			GB_Shader_Emissive    4
#define			GB_Shader_Metal       5


/* 이름 */

#define			GB_Shader_WorldMatrix			"g_matWorld"
#define			GB_Shader_CanvasMatrix			"g_matCanvas"
#define			GB_Shader_CameraBuffer			"g_CameraBuffer"
#define			GB_Shader_CamPosition			"g_CamPosition"
#define			GB_Shader_ViewSpace				"g_matView"
#define			GB_Shader_Projection			"g_matProj"
#define			GB_Shader_ViewXProj				"g_matViewProj"
#define			GB_Shader_Texture				"g_Texture"
#define			GB_Shader_Textures				"g_Textures"
#define			GB_Shader_MixTextureArr			"g_MixTextureArr"
#define			GB_Shader_MaskTexture			"g_MaskTexture"
#define			GB_Shader_MaterialTexture		"g_MaterialTexture"
#define			GB_Shader_Rasterizer			"g_RasterizerState"
#define			GB_Shader_Sampler				"g_SamplerState"
#define			GB_Shader_Blend					"g_BlendState"
#define			GB_Shader_DepthStencil			"g_DepthStencilState"
#define			GB_Shader_LightGroupBuffer		"g_LightGroupBuffer"
#define			GB_Shader_MouseBuffer			"g_MouseBuffer"
#define			GB_Shader_DT					"g_DT"
#define			GB_Shader_UIAlpha				"g_UIAlpha"

/* 콜라이더 상수버퍼 */
#define			ColliderShader_ColliderBufferBox		"g_ColliderBufferBox"
#define			ColliderShader_ColliderBufferRotBox		"g_ColliderBufferRotBox"
#define			ColliderShader_ColliderBufferSphere		"g_ColliderBufferSphere"
#define			ColliderShader_ColliderType				"g_ColliderType"

#define			Shader_Collider						L"Shader_Collider"
#define			Shader_EngineUI						L"Shader_EngineUI"
#define			Shader_Deferred						L"Shader_Deferred"


/* 공용 월드 이름 못찾았을떄 */

/* Effects11 의 IsValid 검사는 CShader 내부 리플렉션 검사로 대체되었다. */
#define SHADER_GB_LINK_FAIL_CHECK(pGB,FileName,VarName)


#pragma endregion 


#define			Proto_Com_VIBuffer_ColliderBox		L"Proto_Component_VIBuffer_ColliderBox"
#define			Proto_Com_VIBuffer_ColliderSpere	L"Proto_Component_VIBuffer_ColliderBox"
#define			Proto_Com_VIBUffer_Rect				L"Proto_Component_VIBuffer_Rect"

#define			Proto_Com_Transform					L"Proto_Component_Transform"
#define			Proto_Com_Collider					L"Proto_Component_Collider"




#pragma region Max_Enum Check



#define			MAX_ENUM_CHECK_RETURN_EFAIL(iMaxEnum , iCheckEnum) \
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return E_FAIL;}

#define			MAX_ENUM_CHECK_RETURN_NULLPTR(iMaxEnum , iCheckEnum)\
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return nullptr;}

#define			MAX_ENUM_CHECK_RETURN_NULL(iMaxEnum , iCheckEnum)\
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return;}


#define			MAX_ENUM_CHECK_RETURN_SOK(iMaxEnum , iCheckEnum)\
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return S_OK;}

#define			MAX_ENUM_CHECK_RETURN_ZERO(iMaxEnum , iCheckEnum)\
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return 0;}

#define			MAX_ENUM_CHECK_RETURN_ONE(iMaxEnum , iCheckEnum)\
				if ( static_cast<int>(iMaxEnum) < static_cast<int>(iCheckEnum)) \
					{return 1;}
#pragma endregion

namespace MyColor
{
#pragma region Color
#define			COLOR_RED				Color(1.f , 0.f  , 0.f, 1.f)
#define			COLOR_GREEN				Color(0.f , 1.f  , 0.1f, 1.f)
#define			COLOR_BLUE				Color(0.f , 0.f  , 1.f, 1.f)
#define			COLOR_WHITE				Color(1.f , 1.f  , 1.f, 1.f)
#define			COLOR_BLACK				Color(0.f , 0.f  , 0.f,1.f)
#define			COLOR_YELLOW			Color(1.f , 1.f  , 0.f, 1.f)			// 노랑
#define			COLOR_ORANGE			Color(1.f , 0.5f , 0.f, 1.f)		// 주황
#define			COLOR_PURPLE			Color(0.6f, 0.f  , 0.8f, 1.f)		// 보라
#define			COLOR_CYAN				Color(0.f , 1.f  , 1.f, 1.f)			// 하늘색
#define			COLOR_MAGENTA			Color(1.f , 0.f  , 1.f, 1.f)			//	Color
#define			COLOR_PINK				Color(1.f , 0.6f , 0.8f, 1.f)
#define			COLOR_MINT				Color(0.6f, 1.f  , 0.8f, 1.f)
#define			COLOR_SKYBLUE			Color(0.4f, 0.7f , 1.f, 1.f)
#define			COLOR_LAVENDER			Color(0.7f, 0.6f , 1.f, 1.f)
#define			COLOR_GOLD				Color(1.f , 0.84f, 0.f, 1.f)
}

#pragma endregion

#pragma region Thread FAil , Success Retrun

//Thread
#define			THREAD_FAIL				1
#define			THREAD_SUCCESS			0

#pragma endregion
#pragma region Engine DLL , Engine Singleton 

#if defined(_WIN32)
#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		__declspec(dllexport)
#else
#define ENGINE_DLL		__declspec(dllimport)
#endif
#else
#define ENGINE_DLL		__attribute__((visibility("default")))
#endif

#define NO_COPY(CLASSNAME)										\
		private:												\
		CLASSNAME(const CLASSNAME&) = delete;					\
		CLASSNAME& operator = (const CLASSNAME&)= delete;		

#define DECLARE_SINGLETON(CLASSNAME)							\
		NO_COPY(CLASSNAME)										\
		private:												\
		static CLASSNAME*	m_pInstance;						\
		public:													\
		static CLASSNAME*	GetInstance( void );				\
		static unsigned int DestroyInstance( void );			

#define IMPLEMENT_SINGLETON(CLASSNAME)							\
		CLASSNAME*	CLASSNAME::m_pInstance = nullptr;			\
		CLASSNAME*	CLASSNAME::GetInstance( void )	{			\
			if(nullptr == m_pInstance) {						\
				m_pInstance = new CLASSNAME;					\
			}													\
			return m_pInstance;									\
		}														\
		unsigned int CLASSNAME::DestroyInstance( void ) {		\
			unsigned int iRefCnt = {0};							\
			if(nullptr != m_pInstance)	{						\
				iRefCnt = m_pInstance->Release();				\
				if(0 == iRefCnt)								\
					m_pInstance = nullptr;						\
			}													\
			return iRefCnt;										\
}

#pragma endregion
#pragma region Initialize_Fail Check


#define ENGINE_WIDEN_IMPL(x)		L##x
#define ENGINE_WIDEN(x)				ENGINE_WIDEN_IMPL(x)
#define INIT_FAILED_CEHCK(pInstance)                                         \
        if (FAILED(pInstance->Initialize()))                                 \
        {                                                                    \
            wchar_t buf[255];                                                \
            swprintf_s(buf, _countof(buf), L"%ls Create is Failed", ENGINE_WIDEN(#pInstance)); \
            MessageBoxW(nullptr, buf, L"System Message", MB_OK);             \
            Engine::Safe_Release(pInstance);                                 \
        }
#pragma endregion
#pragma region XMMath

#define GetLengthVector(tVector3)	XMVectorGetX(XMVector3Length(tVector3)
#define	GetPosition_ToCal()			Get_State_ToCal(STATE::POSITION)							
#define	GetRight_ToCal()			Get_State_ToCal(STATE::RIGHT)							
#define	GetLook_ToCal()				Get_State_ToCal(STATE::LOOK)							
#define	GetUp_ToCal()				Get_State_ToCal(STATE::UP)		

#define	GetPosition_ToStore()		Get_State_ToStore(STATE::POSITION)							
#define	GetRight_ToStore()			Get_State_ToStore(STATE::RIGHT)							
#define	GetLook_ToStore()			Get_State_ToStore(STATE::LOOK)							
#define	GetUp_ToStore()				Get_State_ToStore(STATE::UP)		
#define Get_Right_Defualt()			XMVectorSet(1.f, 1.f, 0.f, 0.f)
#define Get_Up_Defualt()			XMVectorSet(0.f, 1.f, 0.f, 0.f)
#define Get_Look_Defualt()			XMVectorSet(0.f, 0.f, 1.f, 0.f)

#define Vector3Dot(a,b)				XMVectorGetX(XMVector3Dot(a,b))
#define Vector4Dot(a,b)				XMVectorGetX(XMVector4Dot(a,b))

#pragma endregion