#!/bin/bash

# Bash script to install VST3 plugin to the default Linux VST3 folder

# Define source and destination paths
sourcePath="./ai-coustics Demo.vst3"
defaultVST3Path="$HOME/.vst3"

# Check if source file exists
if [ ! -d "$sourcePath" ]; then
    echo "Error: Source VST3 plugin not found at: $sourcePath" >&2
    exit 1
fi

# Create destination directory if it doesn't exist
if [ ! -d "$defaultVST3Path" ]; then
    echo "Creating VST3 directory: $defaultVST3Path"
    mkdir -p "$defaultVST3Path"
fi

# Copy the VST3 plugin to the default location
echo "Installing VST3 plugin..."
if cp -r "$sourcePath" "$defaultVST3Path"; then
    echo "Successfully installed 'ai-coustics Demo.vst3' to $defaultVST3Path"
else
    echo "Error: Failed to install VST3 plugin" >&2
    exit 1
fi

echo "Installation completed successfully!"
