# PowerShell script to create a Windows executable installer
# This script converts the PowerShell installer into a standalone .exe file

#Requires -RunAsAdministrator

param(
    [string]$InstallerScriptPath = ".\install_gui.ps1",
    [string]$OutputName = "Install ai-coustics Plugins.exe"
)

Write-Host "Creating Windows executable installer: $OutputName" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green

# Check if the ps2exe module is available, and install if it's missing
if (-not (Get-Command "ps2exe" -ErrorAction SilentlyContinue)) {
    Write-Host "PS2EXE module not found. Installing..." -ForegroundColor Yellow
    try {
        Install-Module -Name ps2exe -Force -AllowClobber -ErrorAction Stop
        Import-Module ps2exe
        Write-Host "PS2EXE module installed successfully." -ForegroundColor Green
    }
    catch {
        Write-Host "Failed to install PS2EXE module: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "You can install it manually with: Install-Module -Name ps2exe -Force" -ForegroundColor Yellow
        pause
        exit 1
    }
} else {
    Import-Module ps2exe
}

# Convert the PowerShell script to an executable using ps2exe
try {
    $exePath = Join-Path (Get-Location) $OutputName
    Write-Host "Converting PowerShell script '$InstallerScriptPath' to executable..." -ForegroundColor Yellow
    Write-Host "Output: $exePath" -ForegroundColor Yellow

    ps2exe -inputFile $InstallerScriptPath `
           -outputFile $exePath `
           -title "ai-coustics Plugin Installer" `
           -description "ai-coustics Plugin Installer" `
           -company "ai-coustics" `
           -version "1.0.0.0" `
           -copyright "Copyright © $(Get-Date -Format 'yyyy') ai-coustics" `
           -requireAdmin `
           -supportOS `
           -longPaths

    if (Test-Path $exePath) {
        Write-Host ""
        Write-Host "Successfully created: $OutputName" -ForegroundColor Green
        Write-Host ""
        Write-Host "The executable has been configured to automatically request administrator privileges." -ForegroundColor Cyan
        Write-Host "Make sure to distribute the .exe and the 'VST3' folder together." -ForegroundColor Cyan
    } else {
        Write-Host "Error: Failed to create executable." -ForegroundColor Red
    }

} catch {
    Write-Host "Error creating executable: $($_.Exception.Message)" -ForegroundColor Red
}