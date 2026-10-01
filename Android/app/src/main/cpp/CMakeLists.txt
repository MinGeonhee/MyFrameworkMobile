cmake_minimum_required(VERSION 3.22.1)

project(MyFrameworkMobileAndroid LANGUAGES CXX)

set(MY_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(DILIGENT_SOURCE_ROOT "${MY_FRAMEWORK_ROOT}/Library/DiligentEngine")

# OneDrive 경로가 길어서 Diligent/glslang 빌드가 깨질 수 있으므로,
# 빌드 폴더 안에 짧은 NTFS junction을 만들어 사용한다.
if(CMAKE_HOST_WIN32)
    set(DILIGENT_SOURCE_LINK "${CMAKE_BINARY_DIR}/DiligentSource")
    if(NOT EXISTS "${DILIGENT_SOURCE_LINK}")
        file(TO_NATIVE_PATH "${DILIGENT_SOURCE_LINK}" DILIGENT_SOURCE_LINK_NATIVE)
        file(TO_NATIVE_PATH "${DILIGENT_SOURCE_ROOT}" DILIGENT_SOURCE_ROOT_NATIVE)
        execute_process(
            COMMAND cmd /c mklink /J "${DILIGENT_SOURCE_LINK_NATIVE}" "${DILIGENT_SOURCE_ROOT_NATIVE}"
            RESULT_VARIABLE DILIGENT_LINK_RESULT
            OUTPUT_QUIET
            ERROR_VARIABLE DILIGENT_LINK_ERROR
        )
        if(NOT DILIGENT_LINK_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to create short Diligent source link: ${DILIGENT_LINK_ERROR}")
        endif()
    endif()
    set(DILIGENT_SOURCE_ROOT "${DILIGENT_SOURCE_LINK}")
endif()

set(DILIGENT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_DIRECT3D11 ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_DIRECT3D12 ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_OPENGL ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_METAL ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_WEBGPU ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_ARCHIVER ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_SUPER_RESOLUTION ON CACHE BOOL "" FORCE)

add_subdirectory(
    "${DILIGENT_SOURCE_ROOT}"
    "${CMAKE_CURRENT_BINARY_DIR}/DiligentEngine"
)

find_library(ANDROID_LIBRARY android REQUIRED)
find_library(LOG_LIBRARY log REQUIRED)

# 원본 엔진(Windows) 과 같은 include 규칙 : DirectXMath / json / stb / ImGui
set(MY_FRAMEWORK_COMMON_INCLUDES
    "${MY_FRAMEWORK_ROOT}/Engine/Public"
    "${MY_FRAMEWORK_ROOT}/Library/DirectXMath/Inc"
    "${MY_FRAMEWORK_ROOT}/Library/DirectXMath/Stub"
    "${MY_FRAMEWORK_ROOT}/Library/ImGui"
    "${MY_FRAMEWORK_ROOT}/Library/DiligentEngine/DiligentCore"
    "${MY_FRAMEWORK_ROOT}/Library/DiligentEngine/DiligentTools/ThirdParty/stb"
    "${MY_FRAMEWORK_ROOT}/Library/DiligentEngine/DiligentTools/ThirdParty/json/single_include"
)
# __super / MSVC 확장 문법 , 원본 코드의 경고는 끈다
set(MY_FRAMEWORK_COMPILE_OPTIONS -fms-extensions -Wno-everything)

file(GLOB MY_FRAMEWORK_ENGINE_SOURCES CONFIGURE_DEPENDS "${MY_FRAMEWORK_ROOT}/Engine/Private/*.cpp")
add_library(MyFrameworkMobileEngine SHARED ${MY_FRAMEWORK_ENGINE_SOURCES})
set_target_properties(MyFrameworkMobileEngine PROPERTIES OUTPUT_NAME Engine)
target_compile_definitions(MyFrameworkMobileEngine PRIVATE ENGINE_EXPORTS)
target_compile_features(MyFrameworkMobileEngine PRIVATE cxx_std_17)
target_compile_options(MyFrameworkMobileEngine PRIVATE ${MY_FRAMEWORK_COMPILE_OPTIONS})
target_include_directories(MyFrameworkMobileEngine PRIVATE ${MY_FRAMEWORK_COMMON_INCLUDES} "${MY_FRAMEWORK_ROOT}/Engine/Private" "${MY_FRAMEWORK_ROOT}/Engine/Default")
target_link_libraries(MyFrameworkMobileEngine PRIVATE Diligent-GraphicsEngineVk-shared ${ANDROID_LIBRARY} ${LOG_LIBRARY})

file(GLOB MY_FRAMEWORK_GAME_SOURCES CONFIGURE_DEPENDS "${MY_FRAMEWORK_ROOT}/Game/Private/*.cpp")
add_library(MyFrameworkMobileAndroid SHARED NativeMain.cpp ${MY_FRAMEWORK_GAME_SOURCES})
target_compile_features(MyFrameworkMobileAndroid PRIVATE cxx_std_17)
target_compile_options(MyFrameworkMobileAndroid PRIVATE ${MY_FRAMEWORK_COMPILE_OPTIONS})
target_include_directories(MyFrameworkMobileAndroid PRIVATE "${MY_FRAMEWORK_ROOT}/Game/Public" "${MY_FRAMEWORK_ROOT}/Game/Default" ${MY_FRAMEWORK_COMMON_INCLUDES})
target_link_libraries(MyFrameworkMobileAndroid PRIVATE MyFrameworkMobileEngine ${ANDROID_LIBRARY} ${LOG_LIBRARY})
