# Installation Guide

Thank you for testing our ai-coustics speech enhancement demo. There is a standalone version of the plugin available in the `Standalone` folder. You can use it to listen to your microphone signal with headphones or to listen to other signals by routing them into the application using external software like `BlackHole` (macOS) or `Virtual Audio Cable` (Windows).

## Install Audio Plugin

To install the audio plugin, move the `VST3/ai-coustics Demo.vst3` or `AU/ai-coustics Demo.component` file into your system's plugin folder or the folder you selected in your DAW (Digital Audio Workstation).

### Typical Plugin Locations

- **Linux**: `~/.vst3`
- **Windows**: `C:\Program Files\Common Files\VST3`
- **macOS VST**: `/Library/Audio/Plug-Ins/VST3`
- **macOS AU**: `/Library/Audio/Plug-Ins/Components`

Double-check that the folder you selected is also selected in your audio workstation!

### macOS

#### Copy the Plugins Using Terminal

Run these commands from the location of this README file. Open Terminal and execute the following commands:

```bash
sudo cp -r Standalone/ai-coustics\ Demo.app /Applications/
sudo mkdir -p /Library/Audio/Plug-Ins/VST3
sudo cp -r VST3/ai-coustics\ Demo.vst3 /Library/Audio/Plug-Ins/VST3/
sudo cp -r AU/ai-coustics\ Demo.component /Library/Audio/Plug-Ins/Components/
```

#### Resolving "App Is Damaged and Can't Be Opened" Error

If you see this popup when opening the standalone version or the DAW, please follow these instructions:

1. **Remove the app from quarantine** by running these commands in Terminal:

```bash
sudo xattr -r -d com.apple.quarantine /Applications/ai-coustics\ Demo.app
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/VST3/ai-coustics\ Demo.vst3
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/Components/ai-coustics\ Demo.component
```

2. **Open the app or plugin manually**:
    - Ctrl+Click the app or the plugin.
    - Select `Open` from the popup menu. This will allow you to open the app even if the developer is not certified.

### Troubleshooting

- **Plugin Not Showing Up**: Ensure that the plugin is placed in the correct folder and that your DAW is set to scan that folder. If your DAW still does not recognize the plugin, you may need to rescan your plugins. This option is typically found in the settings or preferences menu of your DAW.

Feel free to reach out to us if you encounter any issues or need further assistance.