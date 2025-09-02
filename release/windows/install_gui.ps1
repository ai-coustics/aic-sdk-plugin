# Enhanced PowerShell script with GUI dialogs for installing VST3 plugin
# This script provides a user-friendly installation experience with Windows dialogs

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Function to show message box
function Show-MessageBox {
    param(
        [string]$Message,
        [string]$Title = "ai-coustics Plugin Installer",
        [System.Windows.Forms.MessageBoxButtons]$Buttons = [System.Windows.Forms.MessageBoxButtons]::OK,
        [System.Windows.Forms.MessageBoxIcon]$Icon = [System.Windows.Forms.MessageBoxIcon]::Information
    )
    return [System.Windows.Forms.MessageBox]::Show($Message, $Title, $Buttons, $Icon)
}

# Function to check if running as administrator
function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Function to restart as administrator
function Restart-AsAdmin {
    $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    Start-Process PowerShell -Verb RunAs -ArgumentList $arguments
    exit
}

# Set execution policy for this session
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force

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

# Location of the script
$ScriptDirectory = $PSScriptRoot

# Define source and destination paths
$VST3_SOURCE = $ScriptDirectory + "\VST3\ai-coustics Demo.vst3"
$VST3_DEST = "$env:ProgramFiles\Common Files\VST3"

# Check if source file exists
if (-not (Test-Path $VST3_SOURCE)) {
    $errorMessage = @"
Error: Source VST3 plugin not found at:
$VST3_SOURCE

Please make sure you're running this installer from the correct location with the plugin files present.
"@
    Show-MessageBox -Message $errorMessage -Icon ([System.Windows.Forms.MessageBoxIcon]::Error)
    exit 1
}

# Check if we need administrator privileges
$needsAdmin = $false
try {
    # Try to create a test file in the destination directory
    $testPath = "$VST3_DEST\test_permissions.tmp"
    if (-not (Test-Path $VST3_DEST)) {
        New-Item -ItemType Directory -Path $VST3_DEST -Force -ErrorAction Stop | Out-Null
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
        Write-Host "Creating VST3 directory: $VST3_DEST"
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
    Write-Host "Installing VST3 plugin..."
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

Write-Host "Installation completed successfully!"
