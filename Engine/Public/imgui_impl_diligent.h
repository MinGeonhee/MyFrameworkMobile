#pragma once

/* ------------------------------------------------------------------------------------------- */
/* Dear ImGui 렌더러 백엔드 (원본 imgui_impl_dx11 대체)                                          */
/* Diligent(D3D11 / Vulkan) 위에서 ImGui DrawData 를 그린다.                                     */
/* ImGuiBackendFlags_RendererHasTextures (1.92 동적 폰트 아틀라스) 를 지원한다.                     */
/* 멀티 뷰포트(창 밖으로 꺼내기)는 지원하지 않는다.                                                 */
/* ------------------------------------------------------------------------------------------- */

#include "imgui.h"

namespace Diligent
{
	struct IRenderDevice;
	struct IDeviceContext;
}

IMGUI_IMPL_API bool     ImGui_ImplDiligent_Init(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pContext);
IMGUI_IMPL_API void     ImGui_ImplDiligent_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDiligent_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDiligent_RenderDrawData(ImDrawData* pDrawData);
