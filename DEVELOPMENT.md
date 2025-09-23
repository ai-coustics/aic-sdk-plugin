# Development Guide

## Build from source

The release artefacts will be in `build\AicSdkPlugin_artefacts\Release`.

### Linux/MacOS

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The release artefacts will be in `build/AicSdkPlugin_artefacts/Release`.

To build the macOS installer you have to run the script:

```sh
./scripts/macos/build-installer.sh
```

### Windows

```sh
cmake -B build
cmake --build build --config Release
```

To build the windows installer you will have to install [Inno Setup](https://jrsoftware.org/isinfo.php) and load the file [installer.iss](scripts/windows/installer.iss).

### Linux Dependencies

You can find all necessary Linux dependencies of JUCE in [this document](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md).

### License Key File

If you want to set the license key programmatically, you have to store the key in the following file:

- Linux: `~/.config/aic/aic-sdk-license.txt`
- macOS: ``
- Windows: `C:\Users\<user>\AppData\Roaming\aic\aic-sdk-license.txt`
