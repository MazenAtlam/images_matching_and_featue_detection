@echo off
cd /d "%~dp0"

echo [0/5] Loading environment variables...
if exist .env (
    for /f "usebackq tokens=1,* delims==" %%A in (".env") do (
        set "%%A=%%B"
    )
) else (
    echo [ERROR] .env file not found! Please create it with QT_PREFIX_PATH and WINDEPLOYQT_PATH.
    pause
    exit /b 1
)

echo [1/5] Checking build directory...
if not exist build mkdir build
cd build

echo [2/5] Cleaning up old DLLs to prevent compiler conflicts...
del /q *.dll 2>nul

echo [3/5] Configuring and building the project...
cmake -G "MinGW Makefiles" .. -DCMAKE_PREFIX_PATH="%QT_PREFIX_PATH%"
cmake --build .
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [4/5] Deploying Qt and MinGW DLLs...
"%WINDEPLOYQT_PATH%" --compiler-runtime FeatureDetectionApp.exe

echo [5/5] Launching Application...
start FeatureDetectionApp.exe
