#!/usr/bin/env pwsh
# VST3 Plugin Installer Builder using WiX 6.0
# This script creates a Windows Installer (.msi) for VST3 plugins

param(
    [Parameter(Mandatory=$true)]
    [string]$PluginPath,          # Path to the .vst3 folder

    [Parameter(Mandatory=$true)]
    [string]$PluginName,          # Name of the plugin (for display)

    [Parameter(Mandatory=$true)]
    [string]$Manufacturer,        # Manufacturer/Company name

    [Parameter(Mandatory=$true)]
    [string]$Version,             # Version (e.g., "1.0.0")

    [string]$OutputDir = ".\installer-output",
    [string]$TempDir = ".\temp-installer",
    [switch]$CleanTemp = $true
)

# Validate inputs
if (-not (Test-Path $PluginPath)) {
    Write-Error "Plugin path not found: $PluginPath"
    exit 1
}

if (-not $PluginPath.EndsWith('.vst3')) {
    Write-Error "Plugin path must point to a .vst3 folder"
    exit 1
}

# Extract plugin name from path if not provided explicitly
$PluginFolderName = Split-Path $PluginPath -Leaf

Write-Host "Building VST3 Plugin Installer" -ForegroundColor Green
Write-Host "Plugin: $PluginName" -ForegroundColor Yellow
Write-Host "Path: $PluginPath" -ForegroundColor Yellow
Write-Host "Manufacturer: $Manufacturer" -ForegroundColor Yellow
Write-Host "Version: $Version" -ForegroundColor Yellow

# Create directories
New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

# No Package ID needed - WiX 6.0 will auto-generate

# Create WiX project file
$WixProjContent = @"
<Project Sdk="WixToolset.Sdk/6.0.0">
  <PropertyGroup>
    <OutputType>Package</OutputType>
    <PlatformTarget>x64</PlatformTarget>
  </PropertyGroup>
</Project>
"@

$WixProjPath = Join-Path $TempDir "VST3Plugin.wixproj"
$WixProjContent | Out-File -FilePath $WixProjPath -Encoding UTF8

# Copy plugin files to temp directory for WiX to access
$TempPluginPath = Join-Path $TempDir $PluginFolderName
Copy-Item -Path $PluginPath -Destination $TempPluginPath -Recurse -Force

# Change to temp directory for relative paths
Push-Location $TempDir

# Update TempPluginPath to be relative to current directory
$TempPluginPath = $PluginFolderName

try {
    # Function to recursively generate file entries
    function Get-FilesXml {
        param($Directory, $BasePath, $Indent = 4)

        $IndentStr = " " * $Indent
        $Result = ""

        try {
            $Items = Get-ChildItem $Directory -ErrorAction Stop
            foreach ($Item in $Items) {
                if ($Item.PSIsContainer) {
                    # Directory
                    $DirName = $Item.Name
                    $Result += "$IndentStr<Directory Name=`"$DirName`">`n"
                    $Result += Get-FilesXml -Directory $Item.FullName -BasePath $BasePath -Indent ($Indent + 2)
                    $Result += "$IndentStr</Directory>`n"
                } else {
                    # File - use relative path from temp directory
                    $RelativePath = $Item.FullName.Substring($BasePath.Length + 1).Replace('\', '/')
                    $Result += "$IndentStr<File Source=`"$RelativePath`" />`n"
                }
            }
        }
        catch {
            Write-Warning "Could not process directory: $Directory - $($_.Exception.Message)"
        }

        return $Result
    }

    # Get the base path for relative calculations
    $BasePath = (Get-Location).Path
    # Generate XML for Contents directory (skip the plugin root folder itself)
    $ContentsDir = Join-Path $TempPluginPath "Contents"
    $FilesXml = Get-FilesXml -Directory $ContentsDir -BasePath $BasePath

    # Create WiX source file using string concatenation to avoid here-string issues
    $WxsContent = '<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs">' + "`n"
    $WxsContent += '  <Package Name="' + $PluginName + '"' + "`n"
    $WxsContent += '           Manufacturer="' + $Manufacturer + '"' + "`n"
    $WxsContent += '           Version="' + $Version + '">' + "`n"
    $WxsContent += '' + "`n"
    $WxsContent += '    <!-- Install to VST3 directory -->' + "`n"
    $WxsContent += '    <StandardDirectory Id="CommonFilesFolder">' + "`n"
    $WxsContent += '      <Directory Name="VST3">' + "`n"
    $WxsContent += '        <Directory Name="' + $PluginFolderName + '">' + "`n"
    $WxsContent += '          <Directory Name="Contents">' + "`n"
    $WxsContent += $FilesXml
    $WxsContent += '          </Directory>' + "`n"
    $WxsContent += '        </Directory>' + "`n"
    $WxsContent += '      </Directory>' + "`n"
    $WxsContent += '    </StandardDirectory>' + "`n"
    $WxsContent += '' + "`n"
    $WxsContent += '  </Package>' + "`n"
    $WxsContent += '</Wix>' + "`n"

    $WxsPath = "Package.wxs"
    $WxsContent | Out-File -FilePath $WxsPath -Encoding UTF8

    Write-Host "Generated WiX files:" -ForegroundColor Yellow
    Write-Host "- $WixProjPath" -ForegroundColor Gray
    Write-Host "- $(Join-Path $TempDir $WxsPath)" -ForegroundColor Gray

    Write-Host "Building installer..." -ForegroundColor Yellow

    # Check if .NET SDK is available
    $DotNetVersion = & dotnet --version 2>$null
    if (-not $DotNetVersion) {
        Write-Error ".NET SDK is required but not found. Please install .NET SDK 6.0 or later."
        exit 1
    }

    Write-Host "Using .NET SDK version: $DotNetVersion" -ForegroundColor Green

    # Build the installer
    $BuildResult = & dotnet build "VST3Plugin.wixproj" -c Release --verbosity quiet 2>&1

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed!"
        Write-Host "Build output:" -ForegroundColor Red
        Write-Host $BuildResult -ForegroundColor Red

        # Show the generated WXS content for debugging
        Write-Host "`nGenerated Package.wxs content:" -ForegroundColor Yellow
        Get-Content "Package.wxs" | Write-Host -ForegroundColor Gray
        exit 1
    }

    Write-Host "Build successful!" -ForegroundColor Green

    # Copy the MSI and CAB files to output directory
    $MsiSource = "bin\Release\VST3Plugin.msi"
    $MsiDestination = Join-Path (Split-Path (Get-Location) -Parent) "$OutputDir\$PluginName-$Version.msi"

    if (Test-Path $MsiSource) {
        Copy-Item $MsiSource $MsiDestination -Force

        # Copy any CAB files that are needed for installation
        $CabFiles = Get-ChildItem "bin\Release\*.cab" -ErrorAction SilentlyContinue
        foreach ($CabFile in $CabFiles) {
            $CabDestination = Join-Path (Split-Path (Get-Location) -Parent) "$OutputDir\$($CabFile.Name)"
            Copy-Item $CabFile.FullName $CabDestination -Force
        }

        Write-Host "Installer created: $MsiDestination" -ForegroundColor Green

        # Show file info including CAB files
        $MsiInfo = Get-Item $MsiDestination
        $TotalSize = $MsiInfo.Length
        if ($CabFiles) {
            $CabTotalSize = ($CabFiles | Measure-Object -Property Length -Sum).Sum
            $TotalSize += $CabTotalSize
            Write-Host "MSI size: $([math]::Round($MsiInfo.Length / 1MB, 2)) MB" -ForegroundColor Yellow
            Write-Host "CAB files size: $([math]::Round($CabTotalSize / 1MB, 2)) MB" -ForegroundColor Yellow
        }
        Write-Host "Total installer size: $([math]::Round($TotalSize / 1MB, 2)) MB" -ForegroundColor Yellow

        # Show installation info
        Write-Host "`nInstallation Details:" -ForegroundColor Cyan
        Write-Host "• Target: C:\Program Files\Common Files\VST3\$PluginFolderName" -ForegroundColor White
        Write-Host "• Run the MSI as administrator to install" -ForegroundColor White
        Write-Host "• Keep the MSI and CAB files together for installation" -ForegroundColor White
        Write-Host "• The plugin will be available in VST3-compatible DAWs" -ForegroundColor White

    } else {
        Write-Error "MSI file not found at expected location: $MsiSource"
        Write-Host "Available files in bin\Release:" -ForegroundColor Yellow
        if (Test-Path "bin\Release") {
            Get-ChildItem "bin\Release" | ForEach-Object { Write-Host "  $($_.Name)" -ForegroundColor Gray }
        }
    }

} finally {
    # Return to original directory
    Pop-Location

    # Clean up temp directory
    if ($CleanTemp) {
        Write-Host "Cleaning up temporary files..." -ForegroundColor Yellow
        Remove-Item $TempDir -Recurse -Force -ErrorAction SilentlyContinue
    } else {
        Write-Host "Temporary files preserved in: $TempDir" -ForegroundColor Yellow
    }
}

Write-Host "`nVST3 installer build complete!" -ForegroundColor Green
