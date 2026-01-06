@echo off
REM Build script for WaitForMultipleObjects bug test

REM Check if cl.exe is already in PATH
where cl.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Visual Studio environment already set up.
    goto :build
)

echo Setting up Visual Studio environment...
set VSCMD_SKIP_SENDTELEMETRY=1
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" arm64 >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Failed to set up Visual Studio environment.
    echo Please run this from a Visual Studio Developer Command Prompt.
    exit /b 1
)

:build
echo.
echo Building WaitBugTest...
cl /EHsc /W4 /O2 /nologo WaitBugTest.cpp

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Run WaitBugTest.exe to start the test.
) else (
    echo.
    echo Build failed!
    pause
    exit /b 1
)