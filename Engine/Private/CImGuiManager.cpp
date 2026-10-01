#include "CImGuiManager.h"
#include "CImGuiWindow.h"
#include "imgui_impl_diligent.h"

#ifdef _DEBUG

CImGuiManager::CImGuiManager()
    : m_umapImGuiWindow{}
{
}

CImGuiManager::~CImGuiManager()
{
}

IMPLEMENT_SINGLETON(CImGuiManager)

CImGuiWindow* CImGuiManager::Find_ImGuiWindow(uint32 ID)
{
    auto iter = m_umapImGuiWindow.find(ID);

    if (iter == m_umapImGuiWindow.end())
        return nullptr;
    else
        return iter->second;

    return nullptr;
}

HRESULT CImGuiManager::Add_ImGuiWindow(uint32 ID, CImGuiWindow* pImGuiWindow)
{
    NULLPTR_RETURN_EFAIL(pImGuiWindow);

    CImGuiWindow* pImGui = Find_ImGuiWindow(ID);
    if (pImGui != nullptr)
        return E_FAIL;


    m_umapImGuiWindow.emplace(ID, pImGuiWindow);

    return S_OK;
}

HRESULT CImGuiManager::Initialize_ImGui(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext, ImGui_Style eStyle)
{
    //DPI(화면 배율)에 따라 UI 요소의 크기를 조정하기 위한 스케일 팩터를 구합니다.
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    /* 멀티 뷰포트(메인 창 밖으로 ImGui 창 꺼내기)는 Diligent 백엔드에서 지원하지 않는다 */

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)


    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDiligent_Init(pDevice.Get(), pDeviceContext.Get());

    m_pDevice = pDevice;
    m_pDeviceContext = pDeviceContext;

    return S_OK;
}

void CImGuiManager::Clear_ImGuiFrame()
{
    if (m_umapImGuiWindow.empty() == true)
        return;
    ImGui_ImplDiligent_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void	CImGuiManager::Update(float DT)
{
    if (m_umapImGuiWindow.empty() == true)
        return;
    for (const auto& ImGuiWindow : m_umapImGuiWindow)
    {
        ISNOT_NULLPTR(ImGuiWindow.second)
            ImGuiWindow.second->Update();
    }
}

HRESULT	CImGuiManager::Render()
{
    if (m_umapImGuiWindow.empty() == true)
        return S_OK;

    ImGui::Render();
    ImGui_ImplDiligent_RenderDrawData(ImGui::GetDrawData());

    return S_OK;
}

void CImGuiManager::Free()
{
    __super::Free();

    for (auto& ImGuiWindow : m_umapImGuiWindow)
        Safe_Release(ImGuiWindow.second);


    ImGui_ImplDiligent_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
#endif // DEBUG
