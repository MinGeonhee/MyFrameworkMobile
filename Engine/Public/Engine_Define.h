#pragma once
#if defined(_MSC_VER)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#endif

// 플랫폼 공통 타입 (Windows SDK / Android 대체 정의)
#include "Engine_Platform.h"

// 수학 : 원본과 동일하게 DirectXMath 를 사용한다. (Windows SSE / Android NEON)
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
using namespace DirectX;

// 그래픽 : D3D11 대신 Diligent 공통 인터페이스 (Vulkan / D3D11 백엔드)
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/BlendState.h"
#include "Graphics/GraphicsEngine/interface/DepthStencilState.h"
#include "Graphics/GraphicsEngine/interface/RasterizerState.h"
#include "Graphics/GraphicsEngine/interface/Sampler.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"

#include <nlohmann/json.hpp>

//ImGui (원본과 동일하게 Debug 에서만 사용)
#ifdef _DEBUG
#include "imgui.h"
#if defined(_WIN32)
#include "imgui_impl_win32.h"
#endif
#endif

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>
#include <array>
#include <ctime>
#include <set>
#include <cmath>
#include <random>
#include <mutex>
#include <thread>
#include <atomic>

#include <type_traits>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>

using namespace std::filesystem;


//Smart Pointer
#include <memory>

// My Define Include Header
#include "Engine_Typedef.h"
#include "Engine_Macro.h"
#include "Engine_Enum.h"
#include "Engine_Struct.h"
#include "Engine_Function.h"
#include "Engine_FileIO.h"

// DirectXTK 대체 (PrimitiveBatch / BasicEffect / Colors)
#include "DirectXTK_Compat.h"

// Using Namespace
using namespace std;
//using namespace Engine;
//using namespace DirectX;
