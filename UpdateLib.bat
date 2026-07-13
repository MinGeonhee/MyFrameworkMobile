@echo off
setlocal

set "ROOT=%~dp0"

if not exist "%ROOT%EngineSDK\Inc" mkdir "%ROOT%EngineSDK\Include"
if not exist "%ROOT%EngineSDK\Lib" mkdir "%ROOT%EngineSDK\Library"
if not exist "%ROOT%Client\Bin" mkdir "%ROOT%Client\Bin"

xcopy /y /s /i "%ROOT%Engine\Public\*.*" "%ROOT%EngineSDK\Include\"

if exist "%ROOT%Engine\Bin\Engine.lib" (
    copy /y "%ROOT%Engine\Bin\Engine.lib" "%ROOT%EngineSDK\Include\Engine.lib"
)

if exist "%ROOT%Engine\Bin\Engine.dll" (
    copy /y "%ROOT%Engine\Bin\Engine.dll" "%ROOT%Client\Bin\Engine.dll"
)

endlocal
