# Installation Guide

Thank you for testing our ai-coustics speech enhancement demo.
We recommend using the VST3 or Audio Unit in your favorite DAW, but there is also a standalone version of the plugin in the `Standalone` folder. You can use it to listen to your microphone signal with headphones or to listen to other signals by routing them into the application using external software like `BlackHole` (macOS) or `Virtual Audio Cable` (Windows).

## Install Audio Plugin

### Automatic Installation

The easiest way to install the audio plugin is to open a terminal in this folder and run the install script. Enter your password when you are asked to do so.

On Linux and macOS:

```sh
./install.sh
```

On windows you will have to use Powershell, open it as an administrator with a right-click and run:

```sh
./install.ps1
```

### Manual Installation

To install the audio plugin, move the `VST3/ai-coustics Demo.vst3` or `AU/ai-coustics Demo.component` file into your system's plugin folder or the folder you selected in your DAW (Digital Audio Workstation).

### Typical Plugin Locations

- **Linux**: `~/.vst3`
- **Windows**: `C:\Program Files\Common Files\VST3`
- **macOS VST**: `/Library/Audio/Plug-Ins/VST3`
- **macOS AU**: `/Library/Audio/Plug-Ins/Components`

Double-check that the folder you selected is also selected in your audio workstation!

### Troubleshooting

- **Plugin Not Showing Up**: Ensure that the plugin is placed in the correct folder and that your DAW is set to scan that folder. If your DAW still does not recognize the plugin, you may need to rescan your plugins. This option is typically found in the settings or preferences menu of your DAW.

- **macOS, the Plugin is still not showing up**: It might also be that the plugin is in quarantine. In that case run the following terminal command and enter your password:

```sh
# select the path where you installed the plugins, but those are the defaults
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/VST3/ai-coustics\ Demo.vst3
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/Components/ai-coustics\ Demo.component
```

- **macOS: Resolving "App Is Damaged and Can't Be Opened" Error**: If you see this popup when opening the standalone version, right-click on the application and select open in the context menu. If it still doesn't work, it might be in quarantine. In that case run the following terminal command and enter your password:

```bash
# select the path where you installed the app, but this is the default
sudo xattr -r -d com.apple.quarantine /Applications/ai-coustics\ Demo.app
```

Feel free to reach out to us if you encounter any issues or need further assistance.
