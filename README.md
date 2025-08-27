# ai-coustics SDK Example Plugin

Experience our state-of-the-art speech enhancement technology firsthand with the ai-coustics Demo Plugin - a comprehensive audio plugin that provides instant access to all models in our Speech Enhancement SDK. Built for professional audio workflows, our SDK delivers cutting-edge real-time audio processing that dramatically improves speech clarity, reduces noise, and enhances intelligibility across any audio content.

## What is this Plugin?

The Demo Plugin provides:
- **Real-time audio processing** with instant A/B comparison
- **All model variants** available for testing and evaluation
- **Production-ready C++ code** demonstrating best practices
- **Cross-platform compatibility** (VST3, AU, Standalone)

## Installation

Download the newest release [here](github.com/ai-coustics/aic-sdk-plugin/releases).

### Automatic Installation

The easiest way to install the audio plugin is through the install scripts.

#### macOS

On macOS double click this file:

```sh
./install.command
```

#### Windows
On windows you will have to use Powershell, open it as an administrator with a right-click and run in the downloaded folder:

```sh
./install.ps1
```

#### Linux

On Linux open a terminal in the downloaded folder and run:

```sh
./install.sh
```

We recommend using the VST3 or Audio Unit in your favorite DAW, but there is also a standalone version of the plugin in the `Standalone` folder. You can use it to listen to your microphone signal with headphones or to listen to other signals by routing them into the application using external software like `BlackHole` (macOS) or `Virtual Audio Cable` (Windows).

### Manual Installation

To install the audio plugin, move the `VST3/ai-coustics Demo.vst3` or `AU/ai-coustics Demo.component` file into your system's plugin folder or the folder you selected in your DAW (Digital Audio Workstation).

#### Typical Plugin Locations

- **Linux**: `~/.vst3`
- **Windows**: `C:\Program Files\Common Files\VST3`
- **macOS VST**: `/Library/Audio/Plug-Ins/VST3`
- **macOS AU**: `/Library/Audio/Plug-Ins/Components`

Double-check that the folder you selected is also selected in your audio workstation!

## Troubleshooting

### Plugin Not Showing Up
Ensure that the plugin is placed in the correct folder and that your DAW is set to scan that folder. If your DAW still does not recognize the plugin, you may need to rescan your plugins. This option is typically found in the settings or preferences menu of your DAW.

### macOS: Plugin Still Not Showing Up
It might also be that the plugin is in quarantine. In that case run the following terminal command and enter your password:

```sh
# select the path where you installed the plugins, but those are the defaults
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/VST3/ai-coustics\ Demo.vst3
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/Components/ai-coustics\ Demo.component
```

### macOS: Resolving "App Is Damaged and Can't Be Opened" Error
If you see this popup when opening the standalone version, right-click on the application and select open in the context menu. If it still doesn't work, it might be in quarantine. In that case run the following terminal command and enter your password:

```bash
# select the path where you installed the app, but this is the default
sudo xattr -r -d com.apple.quarantine /Applications/ai-coustics\ Demo.app
```

### Acquire an SDK License Key

To use the SDK, you'll need a license key. Contact our team to receive your time-limited demo key:

- **Email**: [info@ai-coustics.com](mailto:info@ai-coustics.com)
- **Website**: [ai-coustics.com](https://ai-coustics.com)

Once you have your license key, set it as an environment variable or pass it directly to the SDK initialization functions.

### Language Bindings

While this repository contains a C++ integration example, we offer convenient wrappers for popular programming languages:

| Language | Repository | Description |
|----------|------------|-------------|
| **C** | [`aic-sdk-c`](https://github.com/ai-coustics/aic-sdk-c) | Core C interface and foundation |
| **C++** | [`aic-sdk-cpp`](https://github.com/ai-coustics/aic-sdk-cpp) | Modern C++ wrapper with RAII and type safety |
| **Node.js** | [`aic-sdk-node`](https://github.com/ai-coustics/aic-sdk-node) | JavaScript/TypeScript bindings for Node.js |
| **Python** | [`aic-sdk-py`](https://github.com/ai-coustics/aic-sdk-py) | Pythonic interface |
| **Rust** | [`aic-sdk-rs`](https://github.com/ai-coustics/aic-sdk-rs) | Safe Rust Bindings |
| **WebAssembly** | [`aic-sdk-wasm`](https://github.com/ai-coustics/aic-sdk-wasm) | Browser-compatible WebAssembly build |

## Support & Resources

Need help? We're here to assist:

- **Issues**: [GitHub Issues](https://github.com/ai-coustics/aic-sdk-plugin/issues)
- **Technical Support**: [info@ai-coustics.com](mailto:info@ai-coustics.com)

Feel free to reach out to us if you encounter any issues or need further assistance.

## Licenses

The source code in this repo is licensed under the Apache 2.0 wrapper, which does not include the AIC-SDK (downloaded at compile time) and the items in the assets folder, namely [aic-logo.svg](assets/aic_logo.svg) and [aic-font.otf](assets/aic_font.otf), wich are licensed under our [proprietary license](LICENSE.AIC-SDK). You are in no means allowed to copy or redistribute the logo and/or font in the assets folder. JUCE is licensed by the [proprietary JUCE license](https://juce.com/legal/juce-7-license/).

---

Made with ❤️ by the ai-coustics team
