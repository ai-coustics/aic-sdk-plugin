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


## Release

To create a release first check the following things:
- Update version number in `CMakeLists.txt` line 2
- Update version number in `scripts/macos/build-installer.sh` line 7 
- Update version number in `scripts/windows/installer.iss` line 3

Then create a tag on the main branch with the same version number you set in the files before.

## License Key File

If you want to set the license key programmatically, you have to store the key in the following file:

- Linux: `~/.config/aic/aic-sdk-license.txt`
- macOS: `~/Library/aic/aic-sdk-license.txt`
- Windows: `C:\Users\<user>\AppData\Roaming\aic\aic-sdk-license.txt`
