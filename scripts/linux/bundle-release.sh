#!/bin/bash

# Create an archive containing the release folder and install script
mkdir -p aic-sdk-plugin-linux
cp ./scripts/linux/install.sh aic-sdk-plugin-linux/
cp release/* aic-sdk-plugin-linux/
cp -r "build/AicSdkPlugin_artefacts/Release/VST3/ai-coustics Demo.vst3" aic-sdk-plugin-linux/
tar -czf aic-sdk-plugin-linux.tar.gz aic-sdk-plugin-linux/
rm -rf aic-sdk-plugin-linux/
