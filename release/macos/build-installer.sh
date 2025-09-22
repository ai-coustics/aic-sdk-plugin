mkdir -p ./release/macos/package/Library/Audio/Plug-Ins/Components
mkdir -p ./release/macos/package/Library/Audio/Plug-Ins/VST3
cp -r build/AicSdkPlugin_artefacts/Release/AU/ai-coustics\ Demo.component ./release/macos/package/Library/Audio/Plug-Ins/Components
cp -r build/AicSdkPlugin_artefacts/Release/VST3/ai-coustics\ Demo.vst3 ./release/macos/package/Library/Audio/Plug-Ins/VST3
pkgbuild --root ./release/macos/package --identifier com.ai-coustics.demo --version 1.0 --scripts ./release/macos/postinstall.sh ./release/macos/ai-coustics-demo.pkg
