#!/bin/bash

# Bash script to install VST3 and AU plugins to the default macOS audio plugin folders

# Define source and destination paths
VST3_SOURCE="./VST3/ai-coustics Demo.vst3"
AU_SOURCE="./AU/ai-coustics Demo.component"
VST3_DEST="/Library/Audio/Plug-Ins/VST3"
AU_DEST="/Library/Audio/Plug-Ins/Components"

# Check if source files exist
if [ ! -d "$VST3_SOURCE" ]; then
    echo "Error: Source VST3 plugin not found at: $VST3_SOURCE"
    exit 1
fi

if [ ! -d "$AU_SOURCE" ]; then
    echo "Error: Source AU plugin not found at: $AU_SOURCE"
    exit 1
fi

# Create destination directories if they don't exist
if [ ! -d "$VST3_DEST" ]; then
    echo "Creating VST3 directory: $VST3_DEST"
    mkdir -p "$VST3_DEST"
fi

if [ ! -d "$AU_DEST" ]; then
    echo "Creating AU directory: $AU_DEST"
    mkdir -p "$AU_DEST"
fi

# Install VST3 plugin
echo "Installing VST3 plugin..."
if cp -r "$VST3_SOURCE" "$VST3_DEST/"; then
    echo "Successfully installed 'ai-coustics Demo.vst3' to $VST3_DEST"
    xattr -r -d com.apple.quarantine "$VST3_DEST/ai-coustics Demo.vst3" 2>/dev/null || true
else
    echo "Error: Failed to install VST3 plugin"
    exit 1
fi

# Install AU plugin
echo "Installing AU plugin..."
if cp -r "$AU_SOURCE" "$AU_DEST/"; then
    echo "Successfully installed 'ai-coustics Demo.component' to $AU_DEST"
    xattr -r -d com.apple.quarantine "$AU_DEST/ai-coustics Demo.component" 2>/dev/null || true
else
    echo "Error: Failed to install AU plugin"
    exit 1
fi

echo "Installation completed successfully!"
