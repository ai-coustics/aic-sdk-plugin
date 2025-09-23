cmake_minimum_required(VERSION 3.14)
project(AicSdkPlugin VERSION 0.6.3 LANGUAGES C CXX)

# Export compile commands for vscode, vim, zed etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)

include(FetchContent)

# Fetch Juce
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(juce)

# Fetch ai-coustics SDK
set(AIC_SDK_ALLOW_DOWNLOAD ON CACHE BOOL "Allow download ai-coustics C SDK")
FetchContent_Declare(
    aic_sdk
    GIT_REPOSITORY https://github.com/ai-coustics/aic-sdk-cpp.git
    GIT_TAG        0.6.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(aic_sdk)

# JUCE specific options
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

# cmake-format: off
juce_add_plugin(${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  COMPANY_NAME "ai-coustics"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD OFF
  PLUGIN_MANUFACTURER_CODE Aico
  PLUGIN_CODE Aidm
  FORMATS AU VST3 Standalone
  PRODUCT_NAME "ai-coustics Demo"
  MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "We need to access you microphone to enhance your speech"
  NEEDS_WEB_BROWSER FALSE # for Linux
  NEEDS_WEBVIEW2 FALSE) # for Windows
# cmake-format: on

juce_add_binary_data(aic-data SOURCES
    assets/aic_font.otf
    assets/aic_logo.svg
    assets/alert.svg)

target_sources(${PROJECT_NAME} PRIVATE src/PluginEditor.cpp
                                       src/PluginProcessor.cpp
                                       src/LicenseDialog.cpp)

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=0
         JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE juce::juce_audio_utils aic-sdk aic-data
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)
