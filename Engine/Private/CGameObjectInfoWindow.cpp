#include "CGameObjectInfoWindow.h"
#include "CTransform.h"
#include "CGameObject.h"
#include "CImGuiFuntion.h"
#include "Button.h"
#include "Slider_Float.h"
#include "EnumRadioButton.h"
#include "CGameManager.h"

#ifdef _DEBUG



CGameObjectInfoWindow::CGameObjectInfoWindow(HWND hWnd,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CImGuiWindow(hWnd,pDevice,pContext) , m_listTargetGameObject()
{

}

void CGameObjectInfoWindow::Update()
{
    if (ImGui::Begin("GameObject Info Window"))
        MainWindow();

    ImGui::End();
}

void CGameObjectInfoWindow::MainWindow()
{

    GameObjectInfoWindow();

}



void CGameObjectInfoWindow::GameObjectInfoWindow()
{
    MYIMGUI::Button ExceptionButton("Exception GameObject In List", Float2(200, 20));

    /* DeadObject Clean First */
    for (auto iter = m_listTargetGameObject.begin(); iter != m_listTargetGameObject.end();)
    {
        if ((*iter)->Get_Dead())
        {
            iter = m_listTargetGameObject.erase(iter);
        }
        else
            ++iter;
    }

    for (auto& GameObject : m_listTargetGameObject)
    {
        if (m_listTargetGameObject.empty())
        {
            ImGui::Text(" GameObject Info is Empty ");
            return;
        }

        if (GameObject != nullptr)
        {
            wstring Name = GameObject->GetName();

            if (ImGui::TreeNode(WString_To_String(Name).c_str()))
            {
                GameObject->ImGuiUpdate();
                ImGui::TreePop();
            }
            ExceptionButton.SetOnWork([&]() {
                m_listTargetGameObject.remove(GameObject);
                });
            ExceptionButton.Render();
        }
    }
}

void CGameObjectInfoWindow::Add_GameObject(CGameObject* pGameObject)
{
    NULLPTR_RETURN_NULL(pGameObject);

    if (m_listTargetGameObject.empty())
        m_listTargetGameObject.push_back(pGameObject);
    else
    {
        for (auto& GameObject : m_listTargetGameObject)
            if (GameObject == pGameObject) return;

        m_listTargetGameObject.push_back(pGameObject);
    }
}


CGameObjectInfoWindow* CGameObjectInfoWindow::Create(HWND hWnd,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CGameObjectInfoWindow* pGameObjectInfoWindow = new CGameObjectInfoWindow(hWnd, pDevice, pContext);


    if (FAILED(pGameObjectInfoWindow->CImGuiWindow::Initialize()))
    {
        Safe_Release(pGameObjectInfoWindow);
        MSG_BOX("GameObjectInfoWindow is failed to Create");
        return nullptr;
    }


    return pGameObjectInfoWindow;
}

void CGameObjectInfoWindow::Free()
{
    __super::Free();

    m_listTargetGameObject.clear();
}
#endif // _DEBUG
