#!/bin/bash

# Enhanced macOS installer script with better UX for double-click execution
# This script installs VST3, AU plugins, and Standalone application

# Function to display messages in both terminal and dialog
show_message() {
    local message="$1"
    local type="$2"  # "info", "error", or "success"

    echo "$message"

    # Try to show GUI dialog if available
    if command -v osascript >/dev/null 2>&1; then
        case "$type" in
            "error")
                osascript -e "display dialog \"$message\" with title \"ai-coustics Plugin Installer\" buttons {\"OK\"} default button \"OK\" with icon stop"
                ;;
            "success")
                osascript -e "display dialog \"$message\" with title \"ai-coustics Plugin Installer\" buttons {\"OK\"} default button \"OK\" with icon note"
                ;;
            *)
                osascript -e "display dialog \"$message\" with title \"ai-coustics Plugin Installer\" buttons {\"OK\"} default button \"OK\""
                ;;
        esac
    fi
}

# Function to ask for confirmation
ask_confirmation() {
    local message="$1"

    echo "$message"

    if command -v osascript >/dev/null 2>&1; then
        # GUI confirmation
        local result=$(osascript -e "display dialog \"$message\" with title \"ai-coustics Plugin Installer\" buttons {\"Cancel\", \"Install\"} default button \"Install\" with icon question" 2>/dev/null)
        if [[ $? -eq 0 ]] && [[ "$result" == *"Install"* ]]; then
            return 0
        else
            return 1
        fi
    else
        # Terminal confirmation
        read -p "Continue? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            return 0
        else
            return 1
        fi
    fi
}

# Change to the script's directory
cd "$(dirname "$0")"

# Welcome message
show_message "Welcome to the ai-coustics Plugin Installer!

This installer will copy the following to your system:
• VST3 plugin to /Library/Audio/Plug-Ins/VST3
• AU plugin to /Library/Audio/Plug-Ins/Components
• Standalone app to /Applications

Administrator privileges may be required." "info"

# Ask for confirmation
if ! ask_confirmation "Do you want to proceed with the installation?"; then
    show_message "Installation cancelled by user." "info"
    exit 0
fi

# Define source and destination paths
VST3_SOURCE="./VST3/ai-coustics Demo.vst3"
AU_SOURCE="./AU/ai-coustics Demo.component"
STANDALONE_SOURCE="./Standalone/ai-coustics Demo.app"
VST3_DEST="/Library/Audio/Plug-Ins/VST3"
AU_DEST="/Library/Audio/Plug-Ins/Components"
STANDALONE_DEST="/Applications"

# Check if source files exist
missing_files=""
if [ ! -d "$VST3_SOURCE" ]; then
    missing_files="${missing_files}• VST3 plugin: $VST3_SOURCE\n"
fi

if [ ! -d "$AU_SOURCE" ]; then
    missing_files="${missing_files}• AU plugin: $AU_SOURCE\n"
fi

if [ ! -d "$STANDALONE_SOURCE" ]; then
    missing_files="${missing_files}• Standalone app: $STANDALONE_SOURCE\n"
fi

if [ -n "$missing_files" ]; then
    show_message "Error: The following source files were not found:

$missing_files

Please make sure you're running this installer from the correct location with all plugin files present." "error"
    exit 1
fi

# Function to install with proper error handling
install_plugin() {
    local source="$1"
    local dest="$2"
    local name="$3"
    local type="$4"

    echo "Installing $type..."

    # Create destination directory if it doesn't exist
    if [ ! -d "$dest" ]; then
        echo "Creating directory: $dest"
        sudo mkdir -p "$dest" 2>/dev/null || {
            show_message "Error: Could not create directory $dest. Administrator privileges required." "error"
            return 1
        }
    fi

    # Copy the plugin
    if sudo cp -r "$source" "$dest/" 2>/dev/null; then
        echo "Successfully installed '$name' to $dest"

        # Remove quarantine attribute
        sudo xattr -r -d com.apple.quarantine "$dest/$name" 2>/dev/null || true

        # Fix permissions
        sudo chmod -R 755 "$dest/$name" 2>/dev/null || true

        return 0
    else
        show_message "Error: Failed to install $type. Administrator privileges may be required." "error"
        return 1
    fi
}

# Install plugins
success_count=0
total_count=3

echo "Starting installation..."

# Install VST3 plugin
if install_plugin "$VST3_SOURCE" "$VST3_DEST" "ai-coustics Demo.vst3" "VST3 plugin"; then
    ((success_count++))
fi

# Install AU plugin
if install_plugin "$AU_SOURCE" "$AU_DEST" "ai-coustics Demo.component" "AU plugin"; then
    ((success_count++))
fi

# Install Standalone application
if install_plugin "$STANDALONE_SOURCE" "$STANDALONE_DEST" "ai-coustics Demo.app" "Standalone application"; then
    ((success_count++))
fi

# Final message
if [ $success_count -eq $total_count ]; then
    show_message "🎉 Installation completed successfully!

All plugins have been installed:
• VST3 plugin installed to $VST3_DEST
• AU plugin installed to $AU_DEST
• Standalone app installed to $STANDALONE_DEST

You can now use the ai-coustics plugins in your DAW or run the standalone application." "success"
    exit 0
else
    show_message "⚠️ Installation completed with $success_count out of $total_count items installed successfully.

Some plugins may not have been installed correctly. Check the terminal output for details." "error"
    exit 1
fi

# Keep terminal window open if running from double-click
if [ -t 0 ]; then
    echo "Press any key to close..."
    read -n 1
fi
