# ai-coustics SDK Demo Plugin

<div align="center">
    <img src="assets/screenshot.png" alt="Plugin Screenshot" width="600">
</div>

Experience our state-of-the-art speech enhancement technology firsthand with the ai-coustics Demo Plugin - a comprehensive audio plugin that provides instant access to all models in our Speech Enhancement SDK. Built for professional audio workflows, our SDK delivers cutting-edge real-time audio processing that dramatically improves speech clarity, reduces noise, and enhances intelligibility across any audio content.

## What is this Plugin?

The Demo Plugin provides:
- **Real-time audio processing** with instant A/B comparison
- **All model variants** available for testing and evaluation
- **Integration example** demonstrating best practices
- **Cross-platform compatibility** (VST3, AU)

## Installation

Download the newest release [here](https://github.com/ai-coustics/aic-sdk-plugin/releases) and follow the instructions in the [InstallationGuide.md](release/InstallationGuide.md).

### Generate your SDK License Key

To use this SDK, you'll need to generate an **SDK license key** from our [Development Portal](https://developers.ai-coustics.io).

**Please note:** The SDK license key is different from our cloud API product. If you have an API license key for our cloud services, it won't work with the SDK - you'll need to create a separate SDK license key in the portal.

## Build your own Application

If you want to integrate the SDK in your application you can use our language bindings:

| Platform | Repository | Description |
|----------|------------|-------------|
| **C++** | [`aic-sdk-cpp`](https://github.com/ai-coustics/aic-sdk-cpp) | Modern C++ interface with RAII and type safety |
| **Python** | [`aic-sdk-py`](https://github.com/ai-coustics/aic-sdk-py) | Idiomatic Python interface |
| **Rust** | [`aic-sdk-rs`](https://github.com/ai-coustics/aic-sdk-rs) | Safe Rust bindings with zero-cost abstractions |
| **JavaScript/TypeScript** | [`aic-sdk-node`](https://github.com/ai-coustics/aic-sdk-node) | Native bindings for Node.js applications |
| **Web (WASM)** | [`aic-sdk-wasm`](https://github.com/ai-coustics/aic-sdk-wasm) | WebAssembly build for browser applications |

## Support & Resources

Need help? We're here to assist:

- **Issues**: [GitHub Issues](https://github.com/ai-coustics/aic-sdk-plugin/issues)
- **Technical Support**: [info@ai-coustics.com](mailto:info@ai-coustics.com)

Feel free to reach out to us if you encounter any issues or need further assistance.

## Licenses

The source code in this repo is licensed under the Apache 2.0 license, which does not include the AIC-SDK (downloaded at compile time), the JUCE framework, which is licensed under the [proprietary JUCE license](https://juce.com/legal/juce-7-license/), and the items in the assets folder, namely [aic-logo.svg](assets/aic_logo.svg) and [aic-font.otf](assets/aic_font.otf), which are licensed under our [proprietary ai-coustics license](LICENSE.AIC-SDK). You are in no means allowed to copy or redistribute the logo and/or font in the assets folder.

---

Made with ❤️ by the ai-coustics team
