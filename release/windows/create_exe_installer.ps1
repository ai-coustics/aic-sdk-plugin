# PowerShell script to create a Windows executable installer
# This script converts the PowerShell installer into a standalone .exe file

param(
    [string]$OutputName = "Install ai-coustics Plugins.exe"
)

Write-Host "Creating Windows executable installer: $OutputName" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green

# Check if PS2EXE is available
$ps2exeAvailable = Get-Command "ps2exe" -ErrorAction SilentlyContinue

if (-not $ps2exeAvailable) {
    Write-Host "PS2EXE module not found. Installing..." -ForegroundColor Yellow

    # Check if running as administrator for module installation
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    $isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

    if (-not $isAdmin) {
        Write-Host "Administrator privileges required to install PS2EXE module." -ForegroundColor Red
        Write-Host "Please run this script as Administrator or install PS2EXE manually:" -ForegroundColor Red
        Write-Host "Install-Module -Name ps2exe -Force" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Alternatively, you can use the batch file or PowerShell script directly." -ForegroundColor Yellow
        pause
        exit 1
    }

    try {
        Install-Module -Name ps2exe -Force -AllowClobber
        Import-Module ps2exe
        Write-Host "PS2EXE module installed successfully." -ForegroundColor Green
    } catch {
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
        [System.Windows.Forms.MessageBoxButtons]$Buttons = [System.Windows.Forms.MessageBoxButtons]::OK,
        [System.Windows.Forms.MessageBoxIcon]$Icon = [System.Windows.Forms.MessageBoxIcon]::Information
    )
    return [System.Windows.Forms.MessageBox]::Show($Message, $Title, $Buttons, $Icon)
}

function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Restart-AsAdmin {
    $arguments = ""
    Start-Process $MyInvocation.MyCommand.Definition -Verb RunAs
    exit
}

# Welcome message
$welcomeMessage = @"
Welcome to the ai-coustics Plugin Installer!

This installer will copy the VST3 plugin to:
$env:ProgramFiles\Common Files\VST3

Administrator privileges may be required.

Do you want to proceed with the installation?
"@

$result = Show-MessageBox -Message $welcomeMessage -Buttons ([System.Windows.Forms.MessageBoxButtons]::YesNo) -Icon ([System.Windows.Forms.MessageBoxIcon]::Question)

if ($result -eq [System.Windows.Forms.DialogResult]::No) {
    Show-MessageBox -Message "Installation cancelled by user." -Icon ([System.Windows.Forms.MessageBoxIcon]::Information)
    exit 0
}

# Get the directory where the exe is located
$exeDirectory = Split-Path -Parent $MyInvocation.MyCommand.Definition
$VST3_SOURCE = Join-Path $exeDirectory "VST3\ai-coustics Demo.vst3"
$VST3_DEST = "$env:ProgramFiles\Common Files\VST3"

# Check if source file exists
if (-not (Test-Path $VST3_SOURCE)) {
    $errorMessage = @"
Error: Source VST3 plugin not found at:
$VST3_SOURCE

Please make sure the VST3 folder with the plugin file is in the same directory as this installer.

Expected structure:
- Install ai-coustics Plugins.exe
- VST3/
  - ai-coustics Demo.vst3
"@
    Show-MessageBox -Message $errorMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Error)
    exit 1
}

# Check if we need administrator privileges
$needsAdmin = $false
try {
    $testPath = "$VST3_DEST\test_permissions.tmp"
    if (-not (Test-Path $VST3_DEST)) {
        New-Item -ItemType Directory -Path $VST3_DEST -Force -ErrorAction Stop | Out-Null
        Remove-Item -Path $VST3_DEST -Force -ErrorAction SilentlyContinue
        $needsAdmin = $true
    } else {
        New-Item -ItemType File -Path $testPath -Force -ErrorAction Stop | Out-Null
        Remove-Item -Path $testPath -Force -ErrorAction SilentlyContinue
    }
} catch {
    $needsAdmin = $true
}

# If we need admin privileges and don't have them, restart as admin
if ($needsAdmin -and -not (Test-Administrator)) {
    $adminMessage = @"
Administrator privileges are required for installation.

The installer will now restart with administrator privileges.
Please click 'Yes' in the User Account Control dialog.
"@
    Show-MessageBox -Message $adminMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Warning)
    try {
        Restart-AsAdmin
    } catch {
        Show-MessageBox -Message "Failed to restart with administrator privileges. Please run the installer as administrator manually." -Icon ([System.Windows.Forms.MessageBoxIcon]::Error)
        exit 1
    }
}

# Create destination directory if it doesn't exist
try {
    if (-not (Test-Path $VST3_DEST)) {
        New-Item -ItemType Directory -Path $VST3_DEST -Force | Out-Null
    }
} catch {
    $errorMessage = @"
Error: Failed to create destination directory:
$VST3_DEST

Error details: $($_.Exception.Message)
"@
    Show-MessageBox -Message $errorMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Error)
    exit 1
}

# Install the VST3 plugin
try {
    Copy-Item -Path $VST3_SOURCE -Destination $VST3_DEST -Recurse -Force

    $successMessage = @"
🎉 Installation completed successfully!

The ai-coustics Demo VST3 plugin has been installed to:
$VST3_DEST

You can now use the plugin in your DAW.
"@
    Show-MessageBox -Message $successMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Information)

} catch {
    $errorMessage = @"
Error: Failed to install VST3 plugin.

Error details: $($_.Exception.Message)

Please try running the installer as Administrator.
"@
    Show-MessageBox -Message $errorMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Error)
    exit 1
}
'@

# Save the installer script to a temporary file
$tempScriptPath = Join-Path $env:TEMP "ai_coustics_installer_temp.ps1"
$installerScript | Out-File -FilePath $tempScriptPath -Encoding UTF8

# Convert PowerShell script to executable
try {
    $exePath = Join-Path (Get-Location) $OutputName

    Write-Host "Converting PowerShell script to executable..." -ForegroundColor Yellow
    Write-Host "Output: $exePath" -ForegroundColor Yellow

    ps2exe -inputFile $tempScriptPath -outputFile $exePath -iconFile $null -title "ai-coustics Plugin Installer" -description "ai-coustics Plugin Installer" -company "ai-coustics" -version "1.0.0.0" -copyright "Copyright © 2024 ai-coustics" -requireAdmin -supportOS -longPaths

    if (Test-Path $exePath) {
        Write-Host ""
        Write-Host "✅ Successfully created: $OutputName" -ForegroundColor Green
        Write-Host ""
        Write-Host "The executable installer has been created with the following features:" -ForegroundColor Green
        Write-Host "• Native Windows executable (.exe)" -ForegroundColor White
        Write-Host "• GUI dialogs for user interaction" -ForegroundColor White
        Write-Host "• Automatic administrator privilege handling" -ForegroundColor White
        Write-Host "• Professional appearance and metadata" -ForegroundColor White
        Write-Host "• Self-contained (no PowerShell window visible)" -ForegroundColor White
        Write-Host ""
        Write-Host "Distribution:" -ForegroundColor Yellow
        Write-Host "• Include the .exe file and the VST3 folder in the same directory" -ForegroundColor White
        Write-Host "• Users can simply double-click the .exe to install" -ForegroundColor White
        Write-Host "• No additional software required on user machines" -ForegroundColor White

    } else {
        Write-Host "Error: Failed to create executable." -ForegroundColor Red
    }

} catch {
    Write-Host "Error creating executable: $($_.Exception.Message)" -ForegroundColor Red
} finally {
    # Clean up temporary file
    if (Test-Path $tempScriptPath) {
        Remove-Item $tempScriptPath -Force
    }
}

Write-Host ""
Write-Host "Note: The created .exe will look for the VST3 folder in the same directory." -ForegroundColor Cyan
Write-Host "Make sure to distribute both the .exe and the VST3 folder together." -ForegroundColor Cyan
