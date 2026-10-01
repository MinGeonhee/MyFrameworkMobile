#pragma once
#include "Engine_Define.h"


NS_BEGIN(Engine)

class ENGINE_DLL EnumUtils
{
	/* Enum to String / String to Enum */
public:
	template<typename T>
	static constexpr ENUM_CATEGORY GetEnumCategory() { return EnumCategory<T>::value; }

	template<typename T>
	static constexpr const char* GetEnumCategoryName() { return EnumCategory<T>::name; }
public:
	/* GameObject Type */
	static	GAMEOBJECT_TYPE	GAMEOBJECT_TYPE_String_To_Enum(const string& strType);
	static	string GAMEOBJECT_TYPE_Enum_To_String(GAMEOBJECT_TYPE eType);

	/* CAMERA_TYPE */
	static	CAMERA_TYPE	CAMERA_TYPE_String_To_Enum(const string& strType);
	static	string CAMERA_TYPE_Enum_To_String(CAMERA_TYPE eType);

	/* Make_TYPE */
	static MAKE_TYPE MAKE_TYPE_String_To_Enum(const string& strType);
	static string	 MAKE_TYPE_Enum_To_String(MAKE_TYPE eType);

	/* STATE_TYPE  */
	static	STATE_TYPE	STATE_TYPE_String_To_Enum(const string& strType);
	static	string STATE_TYPE_Enum_To_String(STATE_TYPE eType);

	/* PROJECTION_TYPE  */
	static	PROJECTION_TYPE	PROJECTION_TYPE_String_To_Enum(const string& strType);
	static	string PROJECTION_TYPE_Enum_To_String(PROJECTION_TYPE eType);

	/* MOVE TYPE  */
	static	MOVE_TYPE	MOVE_TYPE_String_To_Enum(const string& strType);
	static	string		MOVE_TYPE_Enum_To_String(MOVE_TYPE eType);

	/* MATERIAL_TYPE */
	static  MATERIAL_TYPE	MATERIAL_TYPE_String_To_Enum(const string& strType);
	static  string			MATERIAL_TYPE_Enum_To_String(MATERIAL_TYPE eType);


	/* PROTOTYPE_TYPE */
	static	PROTOTYPE_TYPE	PROTOTYPE_TYPE_String_To_Enum(const string& strType);
	static	string			PROTOTYPE_TYPE_Enum_To_String(PROTOTYPE_TYPE eType);

	/* PROTOTYPE_TYPE */
	static	SCENE_STACK_TYPE	SCENE_STACK_TYPE_String_To_Enum(const string& strType);
	static	string				SCENE_STACK_TYPE_Enum_To_String(SCENE_STACK_TYPE eType);

	/* SCENE_ISPOP */			
	static	SCENE_ISPOP			SCENE_ISPOP_String_To_Enum(const string& strType);
	static	string				SCENE_ISPOP_TYPE_Enum_To_String(SCENE_ISPOP eType);
	/* WINMODE */
	static	WINMODE				WINMODE_String_To_Enum(const string& strType);
	static	string				WINMODE_TYPE_Enum_To_String(WINMODE eType);
	/* Update_Phase */
	static UPDATE_PHASE			UPDATE_PHASE_String_To_Enum(const string& strType);
	static string				UPDATE_PHASE_Enum_To_String(UPDATE_PHASE eType);

	/* Light Type */
	static LIGHT_TYPE			LIGHT_TYPE_String_To_Enum(const string& strType);
	static string				LIGHT_TYPE_Enum_To_String(LIGHT_TYPE eType);
	
	/* SAMPLER_TYPE */
	static SAMPLER_TYPE			SAMPLER_TYPE_String_To_Enum(const string& strType);
	static string				SAMPLER_TYPE_Enum_To_String(SAMPLER_TYPE eType);

	/* Quad Type */
	static Quad_TYPE			Quad_TYPE_String_To_Enum(CONST string& strType);
	static string				Quad_TYPE_Enum_To_String(Quad_TYPE eType);
	/* Cam Depth */
	static string				CAMERA_UPDATE_DEPTH_Enum_To_String(CAMERA_UPDATE_DEPTH eType);
	static string				CAMERA_RENDER_DEPTH_Enum_To_String(CAMERA_RENDER_DEPTH eType);
	static string				UI_DEPTH_Enum_To_String(UI_DEPTH eType);

	/* UPDATETYPE */
	static UPDATE_MODE				UPDATE_MODE_String_To_Enum(CONST string& strType);
	static string					UPDATE_MODE_Enum_To_String(UPDATE_MODE eType);
	
	/* RENDER_GROUP */
	static RENDER_GROUP				RENDER_GROUP_String_To_Enum(CONST string& strType);
	static string					RENDER_GROUP_Enum_To_String(RENDER_GROUP eType);

	/* D3DTS */
	static D3DTS					D3DTS_String_To_Enum(CONST string& strType);
	static string					D3DTS_Enum_To_String(D3DTS eType);

	/*STATE */
	static STATE					STATE_String_To_Enum(CONST string& strType);
	static string					STATE_Enum_To_String(STATE eType);

	/* CALCULATE_TYPE*/
	static CALCULATE_TYPE	CALCULATE_TYPE_String_To_Enum(CONST string& strType);
	static string					CALCULATE_TYPE_Enum_To_String(CALCULATE_TYPE eType);

	/*RAY_TYPE*/
	static RAY_TYPE				RAY_TYPE_TYPE_String_To_Enum(CONST string& strType);
	static string					RAY_TYPE_TYPE_Enum_To_String(RAY_TYPE eType);


	/* Model TYPE */
	static MODEL_TYPE			MODEL_TYPE_String_To_Enum(CONST string& strType);
	static string				MODEL_TYPE_Enum_To_String(MODEL_TYPE eType);

	/* Collider Type */
	static COLLIDER_TYPE		COLLIDER_TYPE_String_To_Enum(CONST string& strType);
	static string				COLLIDER_TYPE_Enum_To_String(COLLIDER_TYPE eType);


	template<class T>
	static string ENUM_TYPE_Enum_To_String(T eType)
	{
		using namespace Engine;

		if constexpr (std::is_same_v<T, WINMODE>)
			return WINMODE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, SCENE_STACK_TYPE>)
			return SCENE_STACK_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, SCENE_ISPOP>)
			return SCENE_ISPOP_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, PROTOTYPE_TYPE>)
			return PROTOTYPE_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, GAMEOBJECT_TYPE>)
			return GAMEOBJECT_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, MAKE_TYPE>)
			return MAKE_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, PROJECTION_TYPE>)
			return PROJECTION_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, CAMERA_TYPE>)
			return CAMERA_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, MOVE_TYPE>)
			return MOVE_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, STATE_TYPE>)
			return STATE_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, UPDATE_MODE>)
			return UPDATE_MODE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, STATE>)
			return STATE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, RAY_TYPE>)
			return RAY_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, LIGHT_TYPE>)
			return LIGHT_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, Quad_TYPE>)
			return Quad_TYPE_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, D3DTS>)
			return D3DTS_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, CAMERA_UPDATE_DEPTH>)
			return CAMERA_UPDATE_DEPTH_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, CAMERA_RENDER_DEPTH>)
			return CAMERA_RENDER_DEPTH_Enum_To_String(eType);

		else if constexpr (std::is_same_v<T, UI_DEPTH>)
			return UI_DEPTH_Enum_To_String(eType);
		
		else if constexpr (std::is_same_v<T, CALCULATE_TYPE>)
			return CALCULATE_TYPE_Enum_To_String(eType);

		else
			return "UNKNOWN";
	}
};


NS_END