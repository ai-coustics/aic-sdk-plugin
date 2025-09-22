#!/bin/bash

mkdir -p ./package/Library/Audio/Plug-Ins/Components
mkdir -p ./package/Library/Audio/Plug-Ins/VST3
cp -r build/AicSdkPlugin_artefacts/Release/AU/ai-coustics\ Demo.component ./package/Library/Audio/Plug-Ins/Components
cp -r build/AicSdkPlugin_artefacts/Release/VST3/ai-coustics\ Demo.vst3 ./package/Library/Audio/Plug-Ins/VST3
pkgbuild --root ./package --identifier com.ai-coustics.demo --version 1.0 --scripts ./scripts/macos/postinstall.sh ./release/aic-sdk-plugin.pkg
