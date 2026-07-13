cmake_minimum_required(VERSION 3.21)

project(MyFrameworkMobile LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Keep the first milestone small. Diligent's sample host supplies the native
# Windows/Android window and swap-chain while our code owns the engine layers.
set(DILIGENT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX ON CACHE BOOL "" FORCE)

add_subdirectory(External/DiligentEngine)

add_library(Engine SHARED
    Engine/Default/dllmain.cpp
    Engine/Public/CBase.h
    Engine/Public/CGameManager.h
    Engine/Public/Engine_Define.h
    Engine/Public/Engine_Enum.h
    Engine/Public/Engine_Macro.h
    Engine/Public/Engine_Struct.h
    Engine/Private/CBase.cpp
    Engine/Private/CGameManager.cpp
    Engine/Private/CGraphicDevice.cpp
    Engine/Private/CGraphicDevice.h
)

target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS)
target_include_directories(Engine
    PUBLIC Engine/Public
    PRIVATE Engine/Private
)
target_link_libraries(Engine PUBLIC Diligent-GraphicsEngine)

set_target_properties(Engine PROPERTIES
    FOLDER "MyFrameworkMobile"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Bin"
)

add_sample_app(Client
    IDE_FOLDER
        MyFrameworkMobile
    SOURCES
        Client/Default/Client.cpp
        Client/Private/CMainApp.cpp
    INCLUDES
        Client/Public/CMainApp.h
)

target_include_directories(Client PRIVATE Client/Public Engine/Public)
target_link_libraries(Client PRIVATE Engine)

set_target_properties(Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Client/Bin"
)

add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/EngineSDK/Inc"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/EngineSDK/Lib"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Public"
            "${CMAKE_CURRENT_SOURCE_DIR}/EngineSDK/Inc"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_LINKER_FILE:Engine>"
            "${CMAKE_CURRENT_SOURCE_DIR}/EngineSDK/Lib/"
)

add_custom_command(TARGET Client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Engine>"
            "$<TARGET_FILE_DIR:Client>/"
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Client)
