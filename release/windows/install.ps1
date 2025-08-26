# PowerShell script to install VST3 plugin to the default Windows VST3 folder

# Define source and destination paths
$sourcePath = ".\VST3\ai-coustics Demo.vst3"
$defaultVST3Path = "$env:ProgramFiles\Common Files\VST3"

# Check if source file exists
if (-not (Test-Path $sourcePath)) {
    Write-Error "Source VST3 plugin not found at: $sourcePath"
    exit 1
}

# Create destination directory if it doesn't exist
if (-not (Test-Path $defaultVST3Path)) {
    Write-Host "Creating VST3 directory: $defaultVST3Path"
    New-Item -ItemType Directory -Path $defaultVST3Path -Force
}

# Copy the VST3 plugin to the default location
try {
    Write-Host "Installing VST3 plugin..."
    Copy-Item -Path $sourcePath -Destination $defaultVST3Path -Recurse -Force
    Write-Host "Successfully installed 'ai-coustics Demo.vst3' to $defaultVST3Path"
}
catch {
    Write-Error "Failed to install VST3 plugin: $($_.Exception.Message)"
    exit 1
}

Write-Host "Installation completed successfully!"
