#include "CLightInfoWindow.h"
#ifdef _DEBUG
#include "CGameManager.h"
#include "MyImGuiHeader.h"
#include "CLightManager.h"
#include "EnumUtils.h"


CLightInfoWindow::CLightInfoWindow(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CImGuiWindow(hWnd, pDevice, pContext) , m_pTargetLight(nullptr) , m_pGameManager(CGameManager::GetInstance())
{
	Safe_AddRef(m_pGameManager);
}


void CLightInfoWindow::Update()
{
	MainWindow();
}

void CLightInfoWindow::MainWindow()
{
	LightListWindow();
	LightInfoWindow();
}

void CLightInfoWindow::LightListWindow()
{
	if (!ImGui::Begin("Light List Window"))
	{
		ImGui::End();
		return;
	}

	for (int i = 0; i < Cast_Uint(LIGHT_TYPE::END); ++i)
	{
		auto LightList = m_pGameManager->LightManager_Get_LightList(static_cast<LIGHT_TYPE>(i));
		string strName = EnumUtils::LIGHT_TYPE_Enum_To_String(static_cast<LIGHT_TYPE>(i));
		int index = 0;
		if (ImGui::TreeNode(strName.c_str()))
		{
			if (LightList.empty())
			{
				ImGui::Text(string(strName + " is Empty").c_str());
				ImGui::TreePop();
			}
			else
			{
				for (auto& Light : LightList)
				{
					ImGui::Text(" Light [ %d ] ", index++);
					CImGuiFuntion::TextMouseClickFunc([&]() {
						m_pTargetLight = Light;
						});
				}
				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
}

void CLightInfoWindow::LightInfoWindow()
{
	if (!ImGui::Begin("Light Info Window"))
	{
		ImGui::End();
		return;
	}

	if (m_pTargetLight == nullptr)
	{
		ImGui::Text(" Target Light is Empty... ");

		ImGui::End();
		return;
	}


	m_pTargetLight->ImGuiUpdate();


	ImGui::End();
}

CLightInfoWindow* CLightInfoWindow::Create(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CLightInfoWindow* pLightInfoWindow = new CLightInfoWindow(hWnd, pDevice, pContext);


	if (FAILED(pLightInfoWindow->CImGuiWindow::Initialize()))
	{
		Safe_Release(pLightInfoWindow);
		MSG_BOX("pLightInfoWindow is failed to Create");
		return nullptr;
	}

	return pLightInfoWindow;
}

void CLightInfoWindow::Free()
{
	Safe_Release(m_pGameManager);

}


#endif

