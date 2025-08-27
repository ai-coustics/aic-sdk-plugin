# macOS Plugin Installer Guide

This directory contains multiple options for installing the ai-coustics plugins on macOS with double-click functionality.

## Available Installer Options

### 1. Simple Command File (`install.command`)
**Best for: Basic users who want a simple double-click solution**

- **File**: `install.command`
- **Usage**: Simply double-click the file
- **Features**: 
  - Runs in Terminal
  - Basic installation process
  - Requires manual confirmation in Terminal

### 2. Enhanced GUI Installer (`install_with_gui.command`)
**Best for: Users who want a better experience with GUI dialogs**

- **File**: `install_with_gui.command`
- **Usage**: Double-click the file
- **Features**:
  - Shows native macOS dialog boxes
  - Better error handling and user feedback
  - Confirmation dialogs before installation
  - Success/error notifications
  - Automatically handles permissions and quarantine removal

### 3. Native macOS App (`Install ai-coustics Plugins.app`)
**Best for: Professional distribution and the most Mac-like experience**

- **File**: `Install ai-coustics Plugins.app`
- **Usage**: Double-click like any Mac application
- **Features**:
  - Native macOS application bundle
  - Professional appearance in Finder
  - Integrated permission requests
  - Native dialog boxes throughout
  - No Terminal window visible to users

## Installation Process

All installers will copy the following files to standard macOS locations:

- **VST3 Plugin**: `ai-coustics Demo.vst3` → `/Library/Audio/Plug-Ins/VST3/`
- **AU Plugin**: `ai-coustics Demo.component` → `/Library/Audio/Plug-Ins/Components/`
- **Standalone App**: `ai-coustics Demo.app` → `/Applications/`

## Prerequisites

1. The following directories must exist in the same folder as the installer:
   ```
   VST3/ai-coustics Demo.vst3
   AU/ai-coustics Demo.component
   Standalone/ai-coustics Demo.app
   ```

2. Administrator privileges will be requested during installation

## Security Notes

### First-Time Usage
Due to macOS Gatekeeper security, users may see a warning when first running the installer:

1. **For .command files**: 
   - Right-click → "Open"
   - Click "Open" in the security dialog

2. **For .app files**:
   - Right-click → "Open"
   - Click "Open" in the security dialog
   - Or go to System Preferences → Security & Privacy → General → "Open Anyway"

### What the Installer Does
- Copies plugin files to system directories
- Removes quarantine attributes (`xattr -r -d com.apple.quarantine`)
- Sets proper file permissions (`chmod -R 755`)
- Creates destination directories if they don't exist

## For Developers

### Creating the App Installer
If you need to recreate the `.app` installer:

```bash
./create_app_installer.sh
```

### Customizing the Installers
- **Plugin names**: Edit the source/destination variables in the scripts
- **Installation paths**: Modify the `*_DEST` variables
- **Dialog messages**: Update the `show_message` function calls

### Code Signing (Recommended for Distribution)
For professional distribution, sign the installers:

```bash
# Sign the .app bundle
codesign --sign "Developer ID Application: Your Name" "Install ai-coustics Plugins.app"

# Sign the .command files
codesign --sign "Developer ID Application: Your Name" install.command
codesign --sign "Developer ID Application: Your Name" install_with_gui.command
```

## Troubleshooting

### Common Issues

1. **Permission Denied**
   - Ensure the installer files are executable: `chmod +x *.command`
   - Run with administrator privileges

2. **Source Files Not Found**
   - Verify the plugin files are in the correct subdirectories
   - Check that file names match exactly (case-sensitive)

3. **Installation Fails**
   - Check available disk space
   - Ensure user has administrator privileges
   - Verify destination directories are writable

### Testing the Installation

After running the installer, verify the plugins are installed:

```bash
# Check VST3
ls -la "/Library/Audio/Plug-Ins/VST3/ai-coustics Demo.vst3"

# Check AU
ls -la "/Library/Audio/Plug-Ins/Components/ai-coustics Demo.component"

# Check Standalone
ls -la "/Applications/ai-coustics Demo.app"
```

## Distribution Recommendations

1. **For End Users**: Use `Install ai-coustics Plugins.app` - most professional
2. **For Beta Testing**: Use `install_with_gui.command` - easier to modify
3. **For Developers**: Keep all versions for different use cases

Include this README file when distributing the installer package to help users choose the right option for their needs.