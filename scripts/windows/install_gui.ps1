# This script installs the ai-coustics VST3 plugin

#Requires -RunAsAdministrator

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Function to show message box
function Show-MessageBox {
    param(
        [string]$Message,
        [string]$Title = "ai-coustics Plugin Installer",
        [System.Windows.Forms.MessageBoxButtons]$Buttons = 'OK',
        [System.Windows.Forms.MessageBoxIcon]$Icon = 'Information'
    )
    return [System.Windows.Forms.MessageBox]::Show($Message, $Title, $Buttons, $Icon)
}

try {
    $scriptDirectory = $null
    if ($PSScriptRoot) {
        # when running as a .ps1 file
        $scriptDirectory = $PSScriptRoot
    } else {
        # when running as a .exe
        $exePath = (Get-Process -Id $PID).Path
        $scriptDirectory = Split-Path -Path $exePath
    }

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

    # Define source and destination paths
    $VST3_SOURCE = Join-Path $scriptDirectory "VST3\ai-coustics Demo.vst3"
    $VST3_DEST = Join-Path $env:ProgramFiles "Common Files\VST3"

    # Check if the source plugin file exists
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
        Write-Host "Creating VST3 directory: $VST3_DEST"
        New-Item -ItemType Directory -Path $VST3_DEST -Force | Out-Null
    }

    # Copy the VST3 plugin
    Copy-Item -Path $VST3_SOURCE -Destination $VST3_DEST -Recurse -Force

    $successMessage = @"
Installation completed successfully!

The ai-coustics Demo VST3 plugin has been installed to:
$VST3_DEST

You can now use the plugin in your DAW.
"@
    Show-MessageBox -Message $successMessage

} catch {
    $errorMessage = @"
Failed to install VST3 plugin.

Error details: $($_.Exception.Message)

Please ensure you have the necessary permissions and try again.
"@
    Show-MessageBox -Message $errorMessage -Icon 'Error'
    exit 1
}

Write-Host "Installation completed successfully!"
exit 0