#!/bin/bash

# Script to create an Automator application for the ai-coustics plugin installer
# This creates a native macOS app that can be double-clicked

APP_NAME="Install ai-coustics Plugins"
SCRIPT_DIR="$(dirname "$0")"
APP_PATH="$SCRIPT_DIR/$APP_NAME.app"

echo "Creating Automator application: $APP_NAME.app"

# Create the app bundle structure
mkdir -p "$APP_PATH/Contents/MacOS"
mkdir -p "$APP_PATH/Contents/Resources"

# Create Info.plist
cat > "$APP_PATH/Contents/Info.plist" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>installer</string>
    <key>CFBundleIconFile</key>
    <string>applet</string>
    <key>CFBundleIdentifier</key>
    <string>com.ai-coustics.plugin-installer</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Install ai-coustics Plugins</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleSignature</key>
    <string>aplt</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.10</string>
    <key>NSHumanReadableCopyright</key>
    <string>Copyright © 2024 ai-coustics. All rights reserved.</string>
    <key>NSMainNibFile</key>
    <string>MainMenu</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
EOF

# Create the main executable script
cat > "$APP_PATH/Contents/MacOS/installer" << 'EOF'
#!/bin/bash

# Get the directory where the app is located
APP_DIR="$(dirname "$(dirname "$(dirname "$0")")")"
INSTALLER_DIR="$(dirname "$APP_DIR")"

# Change to the installer directory
cd "$INSTALLER_DIR"

# Function to show dialog
show_dialog() {
    local message="$1"
    local type="$2"
    local icon="note"

    case "$type" in
        "error") icon="stop" ;;
        "warning") icon="caution" ;;
        "success") icon="note" ;;
    esac

    osascript -e "display dialog \"$message\" with title \"ai-coustics Plugin Installer\" buttons {\"OK\"} default button \"OK\" with icon $icon"
}

# Welcome dialog
result=$(osascript -e 'display dialog "Welcome to the ai-coustics Plugin Installer!\n\nThis will install:\n• VST3 plugin\n• AU plugin\n• Standalone application\n\nAdministrator privileges may be required." with title "ai-coustics Plugin Installer" buttons {"Cancel", "Install"} default button "Install" with icon note' 2>/dev/null)

if [[ $? -ne 0 ]] || [[ "$result" != *"Install"* ]]; then
    show_dialog "Installation cancelled." "info"
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
missing=""
[ ! -d "$VST3_SOURCE" ] && missing="${missing}• VST3 plugin\n"
[ ! -d "$AU_SOURCE" ] && missing="${missing}• AU plugin\n"
[ ! -d "$STANDALONE_SOURCE" ] && missing="${missing}• Standalone app\n"

if [ -n "$missing" ]; then
    show_dialog "Error: Missing plugin files:\n\n${missing}\nPlease ensure all plugin files are present." "error"
    exit 1
fi

# Request administrator privileges upfront
osascript -e 'do shell script "echo Administrator privileges granted" with administrator privileges' >/dev/null 2>&1

if [ $? -ne 0 ]; then
    show_dialog "Administrator privileges are required for installation." "error"
    exit 1
fi

# Function to install plugin
install_plugin() {
    local source="$1"
    local dest="$2"
    local name="$3"

    # Create destination directory
    osascript -e "do shell script \"mkdir -p '$dest'\" with administrator privileges" >/dev/null 2>&1

    # Copy plugin
    osascript -e "do shell script \"cp -r '$source' '$dest'/\" with administrator privileges" >/dev/null 2>&1

    if [ $? -eq 0 ]; then
        # Remove quarantine and fix permissions
        osascript -e "do shell script \"xattr -r -d com.apple.quarantine '$dest/$name' 2>/dev/null || true\" with administrator privileges" >/dev/null 2>&1
        osascript -e "do shell script \"chmod -R 755 '$dest/$name'\" with administrator privileges" >/dev/null 2>&1
        return 0
    else
        return 1
    fi
}

# Install plugins
errors=""

if ! install_plugin "$VST3_SOURCE" "$VST3_DEST" "ai-coustics Demo.vst3"; then
    errors="${errors}• VST3 plugin installation failed\n"
fi

if ! install_plugin "$AU_SOURCE" "$AU_DEST" "ai-coustics Demo.component"; then
    errors="${errors}• AU plugin installation failed\n"
fi

if ! install_plugin "$STANDALONE_SOURCE" "$STANDALONE_DEST" "ai-coustics Demo.app"; then
    errors="${errors}• Standalone app installation failed\n"
fi

# Show final result
if [ -z "$errors" ]; then
    show_dialog "🎉 Installation completed successfully!\n\nAll ai-coustics plugins have been installed and are ready to use in your DAW." "success"
else
    show_dialog "⚠️ Installation completed with errors:\n\n${errors}" "error"
fi
EOF

# Make the executable script executable
chmod +x "$APP_PATH/Contents/MacOS/installer"

# Copy the default Automator icon if available
if [ -f "/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/GenericApplicationIcon.icns" ]; then
    cp "/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/GenericApplicationIcon.icns" "$APP_PATH/Contents/Resources/applet.icns" 2>/dev/null || true
fi

echo "✅ Successfully created: $APP_NAME.app"
echo ""
echo "You can now:"
echo "1. Double-click '$APP_NAME.app' to run the installer"
echo "2. Distribute the entire 'macos' folder to users"
echo "3. The app will show native macOS dialogs for a better user experience"
echo ""
echo "Note: Users may need to right-click and select 'Open' the first time due to Gatekeeper security."
