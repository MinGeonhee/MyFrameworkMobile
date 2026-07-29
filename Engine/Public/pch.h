#pragma once

#include "framework.h"

#pragma region 자료구조 & STL
#include <algorithm>

#include <array>
#include <vector>
#include <list>
#include <string>

#include <map>
#include <unordered_map>

#include <set>
#include <unordered_set>
#pragma endregion
#pragma region 데이터 & 파일
#include <filesystem>
#include <fstream>
#include <istream>
#include <iostream>
#pragma endregion

#include "Engine_Define.h"
#include "Engine_Macro.h"
#include "Engine_Enum.h"
#include "Engine_Funtion.h"
#include "Engine_Struct.h"


#pragma region
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

using Device = Diligent::IRenderDevice;
using Context = Diligent::IDeviceContext;
#pragma endregion


using namespace Diligent;
using namespace std;

