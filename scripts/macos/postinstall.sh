#!/bin/bash

# Set the correct permissions for the installed file
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/Components/ai-coustics\ Demo.component
sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/VST3/ai-coustics\ Demo.vst3

# Optionally, print a message to the user
echo "Installation of ai-coustics Demo.component completed!"
