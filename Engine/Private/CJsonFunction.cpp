#include "CJsonFunction.h"
#include "CCollider.h"
#include "EnumUtils.h"

void CJsonFunction::Write_SRT_Data(nlohmann::ordered_json& Json, const Float4& S, const Float4& R, const Float4& T)
{
	Json["SRT"]["S"] = { { "X" , S.x }   , { "Y" , S.y } , { "Z" , S.z } };
	Json["SRT"]["R"] = { { "X" , R.x }   , { "Y" , R.y } , { "Z" , R.z } };
	Json["SRT"]["T"] = { { "X" , T.x }   , { "Y" , T.y } , { "Z" , T.z } };
}

void CJsonFunction::Read_SRT_Data(const nlohmann::json& Json, Float3& S, Float3& R, Float3& T)
{
	if (!Json.contains("SRT")) return;
	auto& SRTJson = Json["SRT"];
	S.x = SRTJson["S"]["X"].get<float>(); S.y = SRTJson["S"]["Y"].get<float>(); S.z = SRTJson["S"]["Z"].get<float>();
	R.x = SRTJson["R"]["X"].get<float>(); R.y = SRTJson["R"]["Y"].get<float>(); R.z = SRTJson["R"]["Z"].get<float>();
	T.x = SRTJson["T"]["X"].get<float>(); T.y = SRTJson["T"]["Y"].get<float>(); T.z = SRTJson["T"]["Z"].get<float>();
}


void CJsonFunction::Save_ColliderData(nlohmann::ordered_json& SaveColliderJson, COLLIDER_TYPE eColliderType, Float3* Offset, Float3* Extents)
{
	SaveColliderJson["Type"]    = EnumUtils::COLLIDER_TYPE_Enum_To_String(eColliderType);
	SaveColliderJson["Offset"]  = { { "X" , Offset->x } ,{"Y" , Offset->y } ,{"Z" , Offset->z } };
	SaveColliderJson["Extents"] = { { "X" , Extents->x } ,{"Y" , Extents->y } ,{"Z" , Extents->z } };

}

void CJsonFunction::Load_ColliderData(const nlohmann::json& LoadColliderJson, COLLIDER_TYPE& eColliderType  , Float3& Offset, Float3& Scale)
{
	if (LoadColliderJson.empty()) return;

	if( LoadColliderJson.contains("Type"))
		eColliderType = EnumUtils::COLLIDER_TYPE_String_To_Enum(LoadColliderJson["Type"].get<string>());

	if (LoadColliderJson.contains("Offset"))
	{
		Offset.x = LoadColliderJson["Offset"]["X"].get<float>();
		Offset.y = LoadColliderJson["Offset"]["Y"].get<float>();
		Offset.z = LoadColliderJson["Offset"]["Z"].get<float>();
	}

	if (LoadColliderJson.contains("Extents"))
	{
		Scale.x = LoadColliderJson["Extents"]["X"].get<float>();
		Scale.y = LoadColliderJson["Extents"]["Y"].get<float>();
		Scale.z = LoadColliderJson["Extents"]["Z"].get<float>();
	}

	return;

}

void CJsonFunction::Load_SceneNavigationInfo(const nlohmann::json& LoadSceneNavInfoJson,wstring& wstrFindKey, uint32& iPlayerIndex)
{
	wstrFindKey = String_To_WString( LoadSceneNavInfoJson["CellKey"].get<string>());
	iPlayerIndex = LoadSceneNavInfoJson["PlayerCellIndex"].get<uint32>();

	return;
}

