# Windows Plugin Installer Guide

This directory contains multiple options for installing the ai-coustics plugins on Windows with double-click functionality.

## Available Installer Options

### 1. Batch File (`install.bat`)
**Best for: Simple, universal compatibility**

- **File**: `install.bat`
- **Usage**: Simply double-click the file
- **Features**: 
  - Works on all Windows versions
  - No additional software required
  - Command prompt interface
  - Built-in administrator privilege detection
  - User confirmation prompts

### 2. PowerShell Script (`install_gui.ps1`)
**Best for: Modern Windows with GUI dialogs**

- **File**: `install_gui.ps1`
- **Usage**: Double-click the file (may require enabling PowerShell execution)
- **Features**:
  - Native Windows dialog boxes
  - Automatic administrator privilege handling
  - Better error handling and user feedback
  - Professional Windows-style interface
  - Automatic UAC (User Account Control) elevation

### 3. Windows Executable (`Install ai-coustics Plugins.exe`)
**Best for: Professional distribution and easiest user experience**

- **File**: `Install ai-coustics Plugins.exe` (created by `create_exe_installer.ps1`)
- **Usage**: Double-click like any Windows application
- **Features**:
  - Native Windows executable
  - No PowerShell or batch file visibility
  - Professional application metadata
  - Integrated UAC elevation
  - Most user-friendly experience

### 4. Original PowerShell Script (`install.ps1`)
**Best for: Basic PowerShell-based installation**

- **File**: `install.ps1`
- **Usage**: Right-click → "Run with PowerShell"
- **Features**:
  - Simple PowerShell implementation
  - Command-line output
  - Basic error handling

## Installation Process

All installers will copy the VST3 plugin to the standard Windows location:

- **VST3 Plugin**: `ai-coustics Demo.vst3` → `C:\Program Files\Common Files\VST3\`

## Prerequisites

1. The following directory must exist in the same folder as the installer:
   ```
   VST3\ai-coustics Demo.vst3
   ```

2. Administrator privileges will be requested during installation

## Security and Execution Policies

### Batch Files (`.bat`)
- Work immediately without configuration
- May show Windows Defender SmartScreen warning on first run
- Right-click → "More info" → "Run anyway" if blocked

### PowerShell Scripts (`.ps1`)
- May require enabling PowerShell execution policy
- If blocked, open PowerShell as Administrator and run:
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```

### Executable Files (`.exe`)
- Work immediately like any Windows application
- May show Windows Defender SmartScreen warning on first run
- Most user-friendly option

## Creating the Executable Installer

To create the `.exe` version:

1. Open PowerShell as Administrator
2. Navigate to the windows folder
3. Run: `.\create_exe_installer.ps1`

**Requirements:**
- PowerShell 5.0 or later
- Administrator privileges (for installing PS2EXE module)
- Internet connection (for downloading PS2EXE if not installed)

## User Instructions

### For End Users - Recommended Order:

1. **Try the .exe first** (if available): `Install ai-coustics Plugins.exe`
2. **If .exe not available**: `install.bat`
3. **For advanced users**: `install_gui.ps1`

### First-Time Setup:

1. **Download/extract** the installer package
2. **Verify structure**:
   ```
   windows/
   ├── install.bat
   ├── install_gui.ps1
   ├── Install ai-coustics Plugins.exe
   └── VST3/
       └── ai-coustics Demo.vst3
   ```
3. **Double-click** your preferred installer
4. **Allow administrator access** when prompted
5. **Follow on-screen instructions**

## Troubleshooting

### Common Issues

1. **"Access Denied" or "Permission Denied"**
   - Right-click installer → "Run as administrator"
   - Ensure you have administrator privileges on the computer

2. **"File not found" error**
   - Verify the VST3 folder and plugin file are present
   - Check that file names match exactly (case-sensitive)
   - Ensure you're running the installer from the correct directory

3. **PowerShell Execution Policy Error**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

4. **Windows Defender SmartScreen Warning**
   - Click "More info" → "Run anyway"
   - This happens with unsigned executables

### Verifying Installation

After installation, check that the plugin was installed:

1. **Open File Explorer**
2. **Navigate to**: `C:\Program Files\Common Files\VST3`
3. **Verify**: `ai-coustics Demo.vst3` folder exists
4. **Test**: Open your DAW and look for the ai-coustics plugin

### Testing in Different Environments

- **Windows 10/11**: All installers should work
- **Windows 8/8.1**: Use `.bat` or `.exe` versions
- **Windows 7**: Use `.bat` version for best compatibility
- **Corporate environments**: May need IT approval for PowerShell execution

## Distribution Recommendations

### For Software Developers:
- **Professional releases**: Use the `.exe` installer
- **Beta testing**: Use `install_gui.ps1` for easier debugging
- **Universal compatibility**: Include `install.bat` as fallback

### For End Users:
- **Primary choice**: `Install ai-coustics Plugins.exe`
- **Backup option**: `install.bat`
- **Advanced users**: `install_gui.ps1`

### Package Contents:
Always include these files in your distribution:
- At least one installer (preferably `.exe` and `.bat`)
- `VST3/` folder with the plugin
- This README file for user guidance

## Code Signing (Recommended)

For professional distribution, sign your executables to avoid security warnings:

```cmd
signtool sign /f "your-certificate.pfx" /p "password" /t http://timestamp.digicert.com "Install ai-coustics Plugins.exe"
```

This eliminates Windows Defender SmartScreen warnings and increases user trust.