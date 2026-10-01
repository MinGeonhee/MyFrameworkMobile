#include "CImGuiFuntion.h"
#include "CTransform.h"
#include "CGameObject.h"
#include "MyImGuiHeader.h"

#ifdef _DEBUG

void CImGuiFuntion::ColorText(const Color& tColor, const string& str)
{
	ImGui::TextColored(ImVec4(tColor.x , tColor.y, tColor.z, tColor.w), str.c_str());
	return;
}

void CImGuiFuntion::ColorText(const Color& tColor, const wstring& wstr)
{
	std::string str = WString_To_String(wstr);

	ImGui::TextColored(ImVec4(tColor.x, tColor.y, tColor.z, tColor.w), str.c_str());
}

void CImGuiFuntion::RenderMatrix(const Float4x4& tMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ImGui::Text("%.2f",tMatrix.m[i][j]);

			if (j != 3) ImGui::SameLine();
		}
	}
}

void CImGuiFuntion::RenderMatrix(const Matrix& tMatrix)
{
	Float4x4 tMat{};
	XMStoreFloat4x4(&tMat,tMatrix);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ImGui::Text("%.2f", tMat.m[i][j]);

			if (j != 3) ImGui::SameLine();
		}
	}
}

void CImGuiFuntion::RenderTransform(CTransform* pTransform)
{
	Float4 Pos{}, Rot{}, Scale{};
	pTransform->Get_SRT(&Scale, &Rot, &Pos);

	if (ImGui::TreeNode(" Position "))
	{
		Float4 Pos{}, Rot{}, Scale{};
		pTransform->Get_SRT(&Scale, &Rot, &Pos);

		if (ImGui::TreeNode(" Position "))
		{
			CImGuiFuntion::ColorText(COLOR_RED, "Position");
			ImGui::Text(" X : %.2f ", Pos.x);

			ImGui::SameLine();
			ImGui::Text(" Y : %.2f ", Pos.y);

			ImGui::SameLine();
			ImGui::Text(" Z : %.2f ", Pos.z);




			ImGui::TreePop();
		}

		if (ImGui::TreeNode(" Scale "))
		{
			ImGui::Text(" X : %.2f ", Scale.x);

			ImGui::SameLine();
			ImGui::Text(" Y : %.2f ", Scale.y);

			ImGui::SameLine();
			ImGui::Text(" Z : %.2f ", Scale.z);
			ImGui::TreePop();
		}


		if (ImGui::TreeNode(" Rotation "))
		{
			ImGui::Text(" X : %.2f ", Rot.x);

			ImGui::SameLine();
			ImGui::Text(" Y : %.2f ", Rot.y);

			ImGui::SameLine();
			ImGui::Text(" Z : %.2f ", Rot.z);
			ImGui::TreePop();
		}


		if (ImGui::TreeNode("World Matrix"))
		{
			CImGuiFuntion::RenderMatrix(pTransform->Get_matWorld());
			ImGui::TreePop();
		}


	}
	
}

void CImGuiFuntion::RenderGameObjectName(CGameObject* pGameObject)
{
	wstring wstrName = pGameObject->GetName();
	ImGui::Text(" Name : "); ImGui::SameLine();  CImGuiFuntion::ColorText(COLOR_BLUE, wstrName);
}

void CImGuiFuntion::RenderName(const string& str)
{
	ImGui::Text(" Name : "); ImGui::SameLine();  CImGuiFuntion::ColorText(COLOR_BLUE , str.c_str());
}

void CImGuiFuntion::FixName(wstring* pName)
{
	static char NewName[128];
	//ImGui::Text("Fix Name => "); ImGui::SameLine(); 
	ImGui::InputText("FixName", NewName, std::size(NewName));
	MYIMGUI::Button ApplyButton("Apply",40,20);
	
	ApplyButton.SetOnWork([pName]() {
		string strName = NewName; // NewName은 static이므로 여기서 접근 가능
		*pName = String_To_WString(strName);
		});

	ApplyButton.Render();
}

void CImGuiFuntion::RenderName(const wstring& wstr)
{
	std::string str = WString_To_String(wstr);

	ImGui::Text(" Name : "); ImGui::SameLine();  CImGuiFuntion::ColorText(COLOR_BLUE, str.c_str());
}

void CImGuiFuntion::TextMouseClickFunc(std::function<void()> func)
{
	// 현재 줄 전체 영역 가져오기
	ImVec2 min = ImGui::GetItemRectMin();   // 마지막 아이템(=ColorText)의 Min
	ImVec2 max = ImGui::GetItemRectMax();   // 마지막 아이템의 Max

	// min.x를 윈도우 좌측으로 확장 → 줄 전체 커버
	min.x = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x;
	max.x = ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x;

	if (ImGui::IsMouseHoveringRect(min, max))
	{
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(50, 50, 150, 100));
	}
	if (ImGui::IsItemClicked())
	{
		func.operator()();
	}
}

void CImGuiFuntion::RenderIndexAndName(int32 iIndex, wstring& wstrName)
{
	char IndexNum[128];
	sprintf_s(IndexNum, " [ %d ] => ", iIndex);
	ImGui::Text(IndexNum);
	ImGui::SameLine();
	CImGuiFuntion::ColorText(COLOR_BLUE, wstrName.c_str());
}

void CImGuiFuntion::Combo(const char* szLabel, std::string* strListName, uint32 iCurSelectIndex,uint32 iListMaxNum, uint32& iSelectIndex  ,const std::function<void()>& EventFunc)
{
	if (ImGui::BeginCombo(szLabel, strListName[iCurSelectIndex].c_str()))
	{
		for (uint32 i = 0; i < iListMaxNum ; ++i)
		{
			bool isSelectItem = (iCurSelectIndex == i);

			if (ImGui::Selectable(strListName[i].c_str(), isSelectItem))
				if(EventFunc)
					EventFunc();

			if (isSelectItem)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void CImGuiFuntion::DeleteButton(CGameObject* pGameObject)
{
	if (ImGui::Button(" Delete ", ImVec2(32.f, 16.f)))
	{
		/*  단순 Safe_Release 해주는기능 */
		Safe_Release(pGameObject);
	}
}

#endif // _DEBUG

