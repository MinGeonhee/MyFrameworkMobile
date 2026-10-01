#include "EnumUtils.h"

GAMEOBJECT_TYPE EnumUtils::GAMEOBJECT_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "PLAYER")		return GAMEOBJECT_TYPE::PLAYER;
	if (strType == "MONSTER")		return GAMEOBJECT_TYPE::MONSTER;
	if (strType == "NPC")			return GAMEOBJECT_TYPE::NPC;
	if (strType == "PROJECTILE")	return GAMEOBJECT_TYPE::PROJECTILE;
	if (strType == "ITEM")			return GAMEOBJECT_TYPE::ITEM;
	if (strType == "EFFECT")		return GAMEOBJECT_TYPE::EFFECT;
	if (strType == "UI")			return GAMEOBJECT_TYPE::UI;


	return GAMEOBJECT_TYPE::END;
}

string EnumUtils::GAMEOBJECT_TYPE_Enum_To_String(GAMEOBJECT_TYPE eType)
{
	switch (eType)
	{
	case Engine::GAMEOBJECT_TYPE::PLAYER:		return "PLAYER";
	case Engine::GAMEOBJECT_TYPE::MONSTER:		return "MONSTER";
	case Engine::GAMEOBJECT_TYPE::NPC:			return "NPC";
	case Engine::GAMEOBJECT_TYPE::PROJECTILE:	return "PROJECTILE";
	case Engine::GAMEOBJECT_TYPE::ITEM:			return "ITEM";
	case Engine::GAMEOBJECT_TYPE::EFFECT:		return "EFFECT";
	case Engine::GAMEOBJECT_TYPE::UI:			return "UI";
	default:									return "UNKNOWN";
	}

	return "UNKNOWN";
}

CAMERA_TYPE EnumUtils::CAMERA_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "FIRSTPERSON")	return CAMERA_TYPE::FIRSTPERSON;
	if (strType == "THIRDPERSON")	return CAMERA_TYPE::THIRDPERSON;
	if (strType == "FREE")			return CAMERA_TYPE::FREE;

	return CAMERA_TYPE::END;
}

string EnumUtils::CAMERA_TYPE_Enum_To_String(CAMERA_TYPE eType)
{
	switch (eType)
	{
	case CAMERA_TYPE::FIRSTPERSON:	return "FIRSTPERSON";
	case CAMERA_TYPE::THIRDPERSON:	return "THIRDPERSON";
	case CAMERA_TYPE::FREE:			return "FREE";
	default:						return "UNKNOWN";
	}
}

MAKE_TYPE EnumUtils::MAKE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "PROTOTYPE")				return MAKE_TYPE::PROTOTYPE;
	if (strType == "CLONE")					return MAKE_TYPE::CLONE;

	return MAKE_TYPE::END;
}

string EnumUtils::MAKE_TYPE_Enum_To_String(MAKE_TYPE eType)
{
	switch (eType)
	{
	case Engine::MAKE_TYPE::PROTOTYPE:		return "PROTOTYPE";
	case Engine::MAKE_TYPE::CLONE:			return "CLONE";
	default:								return "UNKNOWN";
	}
}

STATE_TYPE EnumUtils::STATE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "RASTERIZER")	return STATE_TYPE::RASTERIZER;
	if (strType == "SAMPLER")		return STATE_TYPE::SAMPLER;
	if (strType == "BLEND")		return STATE_TYPE::BLEND;
	if (strType == "DEAPTHSTENCIL")	return STATE_TYPE::DEAPTHSTENCIL;

	return STATE_TYPE::END;

}

string EnumUtils::STATE_TYPE_Enum_To_String(STATE_TYPE eType)
{
	switch (eType)
	{
	case STATE_TYPE::RASTERIZER:		return "RASTERIZER";
	case STATE_TYPE::SAMPLER:			return "SAMPLER";
	case STATE_TYPE::BLEND:			return "BLEND";
	case STATE_TYPE::DEAPTHSTENCIL:	return "DEAPTHSTENCIL";
	default:							return "UNKNOWN";
	}

	return "UNKNOWN";
}

PROJECTION_TYPE EnumUtils::PROJECTION_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "PERSPECTIVE")	return PROJECTION_TYPE::PERSPECTIVE;
	if (strType == "ORTHOGRAPHIC")	return PROJECTION_TYPE::ORTHOGRAPHIC;

	return PROJECTION_TYPE::END;
}

string EnumUtils::PROJECTION_TYPE_Enum_To_String(PROJECTION_TYPE eType)
{
	switch (eType)
	{
	case PROJECTION_TYPE::PERSPECTIVE:	return "PERSPECTIVE";
	case PROJECTION_TYPE::ORTHOGRAPHIC:	return "ORTHOGRAPHIC";
	default:							return "UNKNOWN";
	}

	return "UNKNOWN";
}

MOVE_TYPE EnumUtils::MOVE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "NORMAL") return MOVE_TYPE::NORMAL;
	if (strType == "SMOOTH") return MOVE_TYPE::SMOOTH;
	if (strType == "UNKNOWN") return MOVE_TYPE::END;

	return MOVE_TYPE::END;
}

string EnumUtils::MOVE_TYPE_Enum_To_String(MOVE_TYPE eType)
{
	switch (eType)
	{
	case Engine::MOVE_TYPE::NORMAL:		return "NORMAL";
	case Engine::MOVE_TYPE::SMOOTH:		return "SMOOTH";
	default:							return "UNKNOWN";
	}

	return "UNKOWN";
}

MATERIAL_TYPE EnumUtils::MATERIAL_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "DIFFUSE")	return MATERIAL_TYPE::DIFFUSE;
	if (strType == "AMBIENT")	return MATERIAL_TYPE::AMBIENT;
	if (strType == "NORMAL")	return MATERIAL_TYPE::NORMAL;
	if (strType == "SPECULAR")	return MATERIAL_TYPE::SPECULAR;
	if (strType == "EMISSIVE")	return MATERIAL_TYPE::EMISSIVE;
	if (strType == "METAL")		return MATERIAL_TYPE::METAL;
	
	return MATERIAL_TYPE::END;
}

string EnumUtils::MATERIAL_TYPE_Enum_To_String(MATERIAL_TYPE eType)
{
	switch (eType)
	{
	case Engine::MATERIAL_TYPE::DIFFUSE:		return "DIFFUSE";
	case Engine::MATERIAL_TYPE::AMBIENT:		return "AMBIENT";
	case Engine::MATERIAL_TYPE::NORMAL:			return "NORMAL";
	case Engine::MATERIAL_TYPE::SPECULAR:		return "SPECULAR";
	case Engine::MATERIAL_TYPE::EMISSIVE:		return "EMISSIVE";
	case Engine::MATERIAL_TYPE::METAL:			return "METAL";
	default:									return "Unknonw";
	}


	return "Unknonw";
}

PROTOTYPE_TYPE EnumUtils::PROTOTYPE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "NORMAL")		return PROTOTYPE_TYPE::GAMEOBJECT;
	if (strType == "SMOOTH")		return PROTOTYPE_TYPE::COMPONENT;
	if (strType == "UNKNOWN")		return PROTOTYPE_TYPE::END;

	return PROTOTYPE_TYPE::END;
}

string EnumUtils::PROTOTYPE_TYPE_Enum_To_String(PROTOTYPE_TYPE eType)
{
	switch (eType)
	{
	case Engine::PROTOTYPE_TYPE::GAMEOBJECT:	return "GAMEOBJECT";
	case Engine::PROTOTYPE_TYPE::COMPONENT:		return "COMPONENT";
	default:									return "UNKNOWN";
	}
}

SCENE_STACK_TYPE EnumUtils::SCENE_STACK_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "NONE")      return SCENE_STACK_TYPE::NONE;
	if (strType == "PUSH")      return SCENE_STACK_TYPE::PUSH;
	if (strType == "REPLACE")   return SCENE_STACK_TYPE::REPLACE;
	if (strType == "CLEAR")     return SCENE_STACK_TYPE::CLEAR;
	if (strType == "END")       return SCENE_STACK_TYPE::END;

	return SCENE_STACK_TYPE::NONE; // default fallback
}

string EnumUtils::SCENE_STACK_TYPE_Enum_To_String(SCENE_STACK_TYPE eType)
{
	switch (eType)
	{
	case SCENE_STACK_TYPE::NONE:      return "NONE";
	case SCENE_STACK_TYPE::PUSH:      return "PUSH";
	case SCENE_STACK_TYPE::REPLACE:   return "REPLACE";
	case SCENE_STACK_TYPE::CLEAR:     return "CLEAR";
	case SCENE_STACK_TYPE::END:       return "END";
	default:                          return "UNKNOWN";
	}
}

SCENE_ISPOP EnumUtils::SCENE_ISPOP_String_To_Enum(const string& strType)
{
	if (strType == "YES") return SCENE_ISPOP::YES;
	if (strType == "NO")  return SCENE_ISPOP::NO;

	return SCENE_ISPOP::NO;
}

string EnumUtils::SCENE_ISPOP_TYPE_Enum_To_String(SCENE_ISPOP eType)
{
	switch (eType)
	{
	case SCENE_ISPOP::YES: return "YES";
	case SCENE_ISPOP::NO:  return "NO";
	default:               return "UNKNOWN";
	}
}

WINMODE EnumUtils::WINMODE_String_To_Enum(const string& strType)
{
	if (strType == "FULL") return WINMODE::FULL;
	if (strType == "WIN")  return WINMODE::WIN;

	return WINMODE::WIN;
}

string EnumUtils::WINMODE_TYPE_Enum_To_String(WINMODE eType)
{
	switch (eType)
	{
	case WINMODE::FULL: return "FULL";
	case WINMODE::WIN:  return "WIN";
	default:            return "UNKNOWN";
	}
}

UPDATE_PHASE EnumUtils::UPDATE_PHASE_String_To_Enum(const string& strType)
{
	if (strType == "PreUpdate")			return UPDATE_PHASE::PreUpdate;
	if (strType == "Update")			return UPDATE_PHASE::Update;
	if (strType == "LateUpdate")		return UPDATE_PHASE::LateUpdate;
	if (strType == "Add_RenderGroup")	return UPDATE_PHASE::Add_RenderGroup;
	if (strType == "Render")			return UPDATE_PHASE::Render;

	return UPDATE_PHASE::END;
}

string EnumUtils::UPDATE_PHASE_Enum_To_String(UPDATE_PHASE eType)
{
	switch (eType)
	{
	case Engine::UPDATE_PHASE::PreUpdate:		return "PreUpdate";
	case Engine::UPDATE_PHASE::Update:			return "Update";
	case Engine::UPDATE_PHASE::LateUpdate:		return "LateUpdate";
	case Engine::UPDATE_PHASE::Add_RenderGroup:	return "Add_RenderGroup";
	case Engine::UPDATE_PHASE::Render:			return "Render";
	default:									return "UNKNOWN";
	}

	return "UNKOWN";
}

LIGHT_TYPE EnumUtils::LIGHT_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "DIRECTIONAL") return LIGHT_TYPE::DIRECTIONAL;
	if (strType == "POINT") return LIGHT_TYPE::POINT;
	


	return LIGHT_TYPE::END;
}

string EnumUtils::LIGHT_TYPE_Enum_To_String(LIGHT_TYPE eType)
{
	switch (eType)
	{
	case Engine::LIGHT_TYPE::DIRECTIONAL:	return "DIRECTIONAL";
	case Engine::LIGHT_TYPE::POINT:				return "POINT";
	default:return "UNKNOWN";
	}
}

SAMPLER_TYPE EnumUtils::SAMPLER_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "WRAP")						return Engine::SAMPLER_TYPE::WRAP;
	if (strType == "CLAMP")						return Engine::SAMPLER_TYPE::CLAMP;
	if (strType == "MIRROR")					return Engine::SAMPLER_TYPE::MIRROR;
	if (strType == "MIRROR_ONCE")				return Engine::SAMPLER_TYPE::MIRROR_ONCE;
	if (strType == "BORDER")					return Engine::SAMPLER_TYPE::BORDER;

	return Engine::SAMPLER_TYPE::END;
}

string EnumUtils::SAMPLER_TYPE_Enum_To_String(SAMPLER_TYPE eType)
{
	switch (eType)
	{
	case Engine::SAMPLER_TYPE::WRAP:			return "WRAP";
	case Engine::SAMPLER_TYPE::CLAMP:			return "CLAMP";
	case Engine::SAMPLER_TYPE::MIRROR:			return "MIRROR";
	case Engine::SAMPLER_TYPE::MIRROR_ONCE:		return "MIRROR_ONCE";
	case Engine::SAMPLER_TYPE::BORDER:			return "BORDER";
	default:									return "Unknown";
	}

	return "Unknown";
}


Quad_TYPE EnumUtils::Quad_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "OVERAY_RENDER")				return Quad_TYPE::OVERAY_RENDER;
	if (strType == "SCREENSHOT_RENDER")		return Quad_TYPE::SCREENSHOT_RENDER;

	return Quad_TYPE::END;
}

string EnumUtils::Quad_TYPE_Enum_To_String(Quad_TYPE eType)
{
	switch (eType)
	{
	case Engine::Quad_TYPE::OVERAY_RENDER:			return "OVERAY_RENDER";
	case Engine::Quad_TYPE::SCREENSHOT_RENDER: return "SCREENSHOT_RENDER";
	default:		return "UNKNOWN";
	}
	return string();
}

string EnumUtils::CAMERA_UPDATE_DEPTH_Enum_To_String(CAMERA_UPDATE_DEPTH eType)
{
	if (eType > CAMERA_UPDATE_DEPTH::END || eType < CAMERA_UPDATE_DEPTH::DEPTH_1)
		return "UNKNOWN";

	return string("CAMERA_UPDATE_DEPTH_" + to_string(Cast_Uint(eType)));
}

string EnumUtils::CAMERA_RENDER_DEPTH_Enum_To_String(CAMERA_RENDER_DEPTH eType)
{
	if (eType > CAMERA_RENDER_DEPTH::END || eType < CAMERA_RENDER_DEPTH::DEPTH_1)
		return "UNKNOWN";

	return string("CAMERA_RENDER_DEPTH_" + to_string(Cast_Uint(eType)));
}

string EnumUtils::UI_DEPTH_Enum_To_String(UI_DEPTH eType)
{
	if (eType > UI_DEPTH::END || eType < UI_DEPTH::DEPTH_OVERAY_SCREENSHOT)
		return "UNKNOWN";

	if (eType == UI_DEPTH::DEPTH_OVERAY_SCREENSHOT) return "UI_DEPTH_OVERAY_SCREENSHOT";

	return string("UI_DEPTH_" + to_string(Cast_Uint(eType)));
}

UPDATE_MODE EnumUtils::UPDATE_MODE_String_To_Enum(const string& strType)
{
	if (strType == "STATIC")				return UPDATE_MODE::STATIC;
	if (strType == "DYNAMIC")		return UPDATE_MODE::DYNAMIC;

	return UPDATE_MODE::END;
}

string EnumUtils::UPDATE_MODE_Enum_To_String(UPDATE_MODE eType)
{
	switch (eType)
	{
	case Engine::UPDATE_MODE::STATIC:		return "STATIC";
	case Engine::UPDATE_MODE::DYNAMIC:		return "DYNAMIC";
	default:												return "UNKNOWN";
	}
}

RENDER_GROUP EnumUtils::RENDER_GROUP_String_To_Enum(const string& strType)
{
	if (strType == "SKYBOX")	return Engine::RENDER_GROUP::SKYBOX;
	if (strType == "PRIORITY")	return Engine::RENDER_GROUP::PRIORITY;
	if (strType == "NONBLEND")	return Engine::RENDER_GROUP::NONBLEND;
	if (strType == "NONLIGHT")	return Engine::RENDER_GROUP::NONLIGHT;
	if (strType == "BLEND")		return Engine::RENDER_GROUP::BLEND;

	return  Engine::RENDER_GROUP::END;
}

string EnumUtils::RENDER_GROUP_Enum_To_String(RENDER_GROUP eType)
{
	switch (eType)
	{
	case Engine::RENDER_GROUP::SKYBOX:			return "SKYBOX";
	case Engine::RENDER_GROUP::PRIORITY:		return "PRIORITY";
	case Engine::RENDER_GROUP::NONBLEND:		return "NONBLEND";
	case Engine::RENDER_GROUP::NONLIGHT:		return "NONLIGHT";
	case Engine::RENDER_GROUP::BLEND:			return "BLEND";
	default:									return "UNKNOWN";
	}

	return "UNKNOWN";
}

D3DTS EnumUtils::D3DTS_String_To_Enum(const string& strType)
{
	if (strType == "VIEW")				return D3DTS::VIEW;
	if (strType == "PROJ")		return D3DTS::PROJ;

	return D3DTS::END;
}

string EnumUtils::D3DTS_Enum_To_String(D3DTS eType)
{
	switch (eType)
	{
	case Engine::D3DTS::VIEW:		return "VIEW";
	case Engine::D3DTS::PROJ:		return "PROJ";
	default:								return "UNKNOWN";
	}
}

STATE EnumUtils::STATE_String_To_Enum(const string& strType)
{
	if (strType == "RIGHT")		return STATE::RIGHT;
	if (strType == "UP")			return STATE::UP;
	if (strType == "LOOK")		return STATE::LOOK;
	if (strType == "POSITION")	return STATE::POSITION;

	return STATE::END;
}
string EnumUtils::STATE_Enum_To_String(STATE eType)
{
	switch (eType)
	{
	case Engine::STATE::RIGHT:			return "RIGHT";
	case Engine::STATE::UP:					return "UP";
	case Engine::STATE::LOOK:				return "LOOK";
	case Engine::STATE::POSITION:		return "POSITION";
	default:										return "UNKNOWN";
	}
}


CALCULATE_TYPE EnumUtils::CALCULATE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "DEFAULT")				return CALCULATE_TYPE::DEFAULT;
	if (strType == "ADD")						return CALCULATE_TYPE::ADD;
	if (strType == "SUB")						return CALCULATE_TYPE::SUB;
	if (strType == "MUL")						return CALCULATE_TYPE::MUL;
	if (strType == "DIV")						return CALCULATE_TYPE::DIV;


	return CALCULATE_TYPE::END;
}

string EnumUtils::CALCULATE_TYPE_Enum_To_String(CALCULATE_TYPE eType)
{
	switch (eType)
	{
	case Engine::CALCULATE_TYPE::DEFAULT:	return "DEFAULT";
	case Engine::CALCULATE_TYPE::ADD:		return "ADD";
	case Engine::CALCULATE_TYPE::SUB:		return "SUB";
	case Engine::CALCULATE_TYPE::MUL:		return "MUL";
	case Engine::CALCULATE_TYPE::DIV:			return "DIV";
	default:												return "UNKNOWN";
	}
}

RAY_TYPE EnumUtils::RAY_TYPE_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "TERRAIN") return RAY_TYPE::TERRAIN;

	return RAY_TYPE::END;
}

string EnumUtils::RAY_TYPE_TYPE_Enum_To_String(RAY_TYPE eType)
{
	switch (eType)
	{
	case Engine::RAY_TYPE::TERRAIN:	return "TERRAIN";
	default:						return "UNKNOWN";
	}
}

MODEL_TYPE EnumUtils::MODEL_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "NONANIM")			return Engine::MODEL_TYPE::NONANIM;
	if (strType == "ANIM")				return Engine::MODEL_TYPE::ANIM;

	return MODEL_TYPE::END;
}

string EnumUtils::MODEL_TYPE_Enum_To_String(MODEL_TYPE eType)
{
	switch (eType)
	{
	case Engine::MODEL_TYPE::NONANIM:	return "NONANIM";
	case Engine::MODEL_TYPE::ANIM:		return "ANIM";
	default:							return "UNKNOWN";
	}
	return "UNKNOWN";
}

COLLIDER_TYPE EnumUtils::COLLIDER_TYPE_String_To_Enum(const string& strType)
{
	if (strType == "SPHERE")		return Engine::COLLIDER_TYPE::SPHERE;
	if (strType == "AABB")			return Engine::COLLIDER_TYPE::AABB;
	if (strType == "OBB")		return Engine::COLLIDER_TYPE::OBB;

	return Engine::COLLIDER_TYPE::END;
}

string EnumUtils::COLLIDER_TYPE_Enum_To_String(COLLIDER_TYPE eType)
{
	switch (eType)
	{
	case Engine::COLLIDER_TYPE::SPHERE:			return "SPHERE";
	case Engine::COLLIDER_TYPE::AABB:			return "AABB";
	case Engine::COLLIDER_TYPE::OBB:			return "OBB";
	default:									return "UNKNOWN";
	}

	return "UNKNOWN";
}

