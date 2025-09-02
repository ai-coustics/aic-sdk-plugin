# PowerShell script to create a Windows executable installer
# This script converts the PowerShell installer into a standalone .exe file

#Requires -RunAsAdministrator

param(
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

# Create the embedded installer script
$installerScript = @'
# Embedded ai-coustics Plugin Installer
# This is a standalone executable created from PowerShell

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

function Show-MessageBox {
    param(
        [string]$Message,
        [string]$Title = "ai-coustics Plugin Installer",
        [System.Windows.Forms.MessageBoxButtons]$Buttons = 'OK',
        [System.Windows.Forms.MessageBoxIcon]$Icon = 'Information'
    )
    return [System.Windows.Forms.MessageBox]::Show($Message, $Title, $Buttons, $Icon)
}

# Welcome message
$welcomeMessage = @"
Welcome to the ai-coustics Plugin Installer!

This installer will copy the VST3 plugin to:
$env:ProgramFiles\Common Files\VST3

Do you want to proceed with the installation?
"@

$result = Show-MessageBox -Message $welcomeMessage -Buttons 'YesNo' -Icon 'Question'

if ($result -ne 'Yes') {
    Show-MessageBox -Message "Installation cancelled by user."
    exit 0
}

# Install
try {
    # Get the directory where the .exe is running
    $exePath = (Get-Process -Id $PID).Path
    $exeDirectory = Split-Path -Path $exePath

    # Define source and destination paths
    $VST3_SOURCE = Join-Path $exeDirectory "VST3\ai-coustics Demo.vst3"
    $VST3_DEST = Join-Path $env:ProgramFiles "Common Files\VST3"

    # Check if source file exists
    if (-not (Test-Path $VST3_SOURCE)) {
        $errorMessage = @"
Error: Source VST3 plugin not found at:
$VST3_SOURCE

Please make sure the 'VST3' folder with the plugin is in the same directory as this installer.
"@
        Show-MessageBox -Message $errorMessage -Icon 'Error'
        exit 1
    }

    # Create destination directory if it doesn't exist
    if (-not (Test-Path $VST3_DEST)) {
        New-Item -ItemType Directory -Path $VST3_DEST -Force | Out-Null
    }

    # Copy the VST3 plugin
    Copy-Item -Path $VST3_SOURCE -Destination $VST3_DEST -Recurse -Force

    $successMessage = @"
🎉 Installation completed successfully!

The ai-coustics Demo VST3 plugin has been installed to:
$VST3_DEST

You can now use the plugin in your DAW.
"@
    Show-MessageBox -Message $successMessage

} catch {
    $errorMessage = @"
Error: Failed to install VST3 plugin.

Error details: $($_.Exception.Message)

Please ensure you have the necessary permissions and try again.
"@
    Show-MessageBox -Message $errorMessage -Icon 'Error'
    exit 1
}
'@

# Save the installer script to a temporary file
$tempScriptPath = Join-Path $env:TEMP "ai_coustics_installer_temp.ps1"
$installerScript | Out-File -FilePath $tempScriptPath -Encoding UTF8

# Convert the PowerShell script to an executable using ps2exe
try {
    $exePath = Join-Path (Get-Location) $OutputName
    Write-Host "Converting PowerShell script to executable..." -ForegroundColor Yellow
    Write-Host "Output: $exePath" -ForegroundColor Yellow

    ps2exe -inputFile $tempScriptPath `
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
        Write-Host "✅ Successfully created: $OutputName" -ForegroundColor Green
        Write-Host ""
        Write-Host "The executable has been configured to automatically request administrator privileges." -ForegroundColor Cyan
        Write-Host "Make sure to distribute the .exe and the 'VST3' folder together." -ForegroundColor Cyan
    } else {
        Write-Host "Error: Failed to create executable." -ForegroundColor Red
    }

} catch {
    Write-Host "Error creating executable: $($_.Exception.Message)" -ForegroundColor Red
} finally {
    # Clean up the temporary script file
    if (Test-Path $tempScriptPath) {
        Remove-Item $tempScriptPath -Force
    }
}
