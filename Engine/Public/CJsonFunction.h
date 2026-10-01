#pragma once
#include "Engine_Define.h"

namespace Engine
{
	class CCollider;
}


NS_BEGIN(Engine)

class ENGINE_DLL CJsonFunction 
{
public:
	static void Write_SRT_Data(nlohmann::ordered_json& Json , const Float4& S, const Float4& R, const Float4& T);
	static void Read_SRT_Data(const nlohmann::json& Json, Float3& S, Float3& R, Float3& T);

	static void Save_ColliderData(nlohmann::ordered_json& SaveColliderJson, COLLIDER_TYPE eColliderType, Float3* Offset, Float3* Extents);
	static void	Load_ColliderData(const nlohmann::json& LoadColliderJson, COLLIDER_TYPE& eColliderType,Float3& Offset , Float3& Scale);

	static void Load_SceneNavigationInfo(const nlohmann::json& LoadSceneNavInfoJson, wstring& wstrFindKey, uint32& iPlayerIndex);

};

NS_END