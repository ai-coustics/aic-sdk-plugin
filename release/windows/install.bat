@echo off
setlocal enabledelayedexpansion

REM Batch script to install VST3 plugin to the default Windows VST3 folder
REM This script can be double-clicked to run

echo =========================================
echo   ai-coustics Plugin Installer
echo =========================================
echo.

REM Define source and destination paths
set "VST3_SOURCE=.\VST3\ai-coustics Demo.vst3"
set "VST3_DEST=%ProgramFiles%\Common Files\VST3"

REM Check if source file exists
if not exist "%VST3_SOURCE%" (
    echo ERROR: Source VST3 plugin not found at: %VST3_SOURCE%
    echo.
    echo Please make sure you're running this installer from the correct location
    echo with the plugin files present.
    echo.
    pause
    exit /b 1
)

echo Found VST3 plugin: %VST3_SOURCE%
echo Target location: %VST3_DEST%
echo.

REM Ask for confirmation
set /p "confirm=Do you want to install the ai-coustics VST3 plugin? (Y/N): "
if /i not "%confirm%"=="Y" (
    echo Installation cancelled.
    echo.
    pause
    exit /b 0
)

echo.
echo Installing VST3 plugin...

REM Create destination directory if it doesn't exist
if not exist "%VST3_DEST%" (
    echo Creating VST3 directory: %VST3_DEST%
    mkdir "%VST3_DEST%" 2>nul
    if errorlevel 1 (
        echo ERROR: Failed to create directory. Administrator privileges may be required.
        echo.
        echo Try running this installer as Administrator:
        echo 1. Right-click on install.bat
        echo 2. Select "Run as administrator"
        echo.
        pause
        exit /b 1
    )
)

REM Copy the VST3 plugin
xcopy "%VST3_SOURCE%" "%VST3_DEST%\" /E /I /Y >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to install VST3 plugin.
    echo Administrator privileges may be required.
    echo.
    echo Try running this installer as Administrator:
    echo 1. Right-click on install.bat
    echo 2. Select "Run as administrator"
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Installation Completed Successfully!
echo ========================================
echo.
echo The ai-coustics Demo VST3 plugin has been installed to:
echo %VST3_DEST%
echo.
echo You can now use the plugin in your DAW.
echo.
pause
