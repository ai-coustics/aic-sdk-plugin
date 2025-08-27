# Development Guide

## Build from source

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The release artefacts will be in `build/AicSdkPlugin_artefacts/Release`.

### Linux Dependencies

You can find all necessary Linux dependencies of JUCE in [this document](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md).
