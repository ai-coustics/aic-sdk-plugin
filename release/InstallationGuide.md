# Installation Guide

Thank you for testing our **ai-coustics speech enhancement demo**.
This is an audio plugin that can be opened in your favorite DAW (Digital Audio Workstation).

If you do not have a DAW installed, we recommend installing [REAPER](https://www.reaper.fm/), which can be evaluated for free.

---

## Install Audio Plugin

### macOS

1. Locate the file `aic-sdk-plugin.pkg` and double-click it to start the installer.
2. You will see a warning saying the developer cannot be verified. Click **Done** to close this dialog.
3. Open **System Settings** from the Apple menu or Dock.
4. Go to **Privacy & Security** in the sidebar.
5. In the **Security** section, you should see a message about the blocked installer. Click **Open Anyway**.
6. When prompted again, click **Open Anyway**.
7. Enter your administrator password and follow the installer instructions to complete installation.

The plugin will now be installed and available as `ai-coustics Demo` in your DAW.

* AU Install Location: `Library/Audio/Plug-Ins/Components/ai-coustics Demo.component`
* VST3 Install Location: `/Library/Audio/Plug-Ins/VST3/ai-coustics Demo.vst3`
* [Apple Documentation](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unknown-developer-mh40616/mac)

---

### Windows

1. Locate the file `aic-sdk-plugin-windows.exe` and double-click it to start the installer.
2. The first time you run it, you will see a **Windows protected your PC** warning.

   * Click **More info** → **Run anyway**.
3. Confirm the **User Account Control** dialog by clicking **Yes**.
4. Follow the installer instructions. If it warns that the `Common Files` folder already exists, simply proceed.

The plugin will now be installed and available as `ai-coustics Demo` in your DAW.

* VST3 Install Location: `C:\Program Files\Common Files\VST3\`

---

### Linux

1. Open a terminal in the directory containing the installer files.

2. Run the installation script:

   ```sh
   ./install.sh
   ```

3. The plugin will be installed to the default system location.

The plugin will now be available as `ai-coustics Demo` in your DAW.

* VST3 Install Location: `~/.vst3/`

---

## License Key

When you open the plugin in your DAW for the first time, a license key dialog will appear.

1. Paste the provided license key into the text input field.
2. Click **Activate**.

You can also place the license key manually in the following file paths:

* **Linux**: `~/.config/aic/aic-sdk-license.txt`
* **macOS**: `~/Library/aic/aic-sdk-license.txt`
* **Windows**: `C:\Users\<user>\AppData\Roaming\aic\aic-sdk-license.txt`

---

If you run into any issues, feel free to reach out to us for support.
