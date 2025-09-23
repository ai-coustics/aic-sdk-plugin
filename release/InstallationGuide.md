# Installation Guide

Thank you for testing our ai-coustics speech enhancement demo. This is an audio plugin that can be opened in your favorite DAW (Digital Audio Workstation).
If you do not have a DAW installed, we recommend installing [REAPER](https://www.reaper.fm/), because it can be evaluated for free.

## Install Audio Plugin

### macOS

1. Start the installer by double-clicking on the `aic-sdk-plugin.pkg` file.

2. The first time you open it, you will see a security warning saying the developer cannot be verified. Click `Done` to close this dialog.

3. Open **System Settings** from the Apple menu or Dock.

4. Click on **Privacy & Security** in the sidebar.

5. Scroll down to the **Security** section. You should see a message about the blocked installer with an **Open anyways** button next to it. Click **Open anyway**.

6. This time in the popup click **Open anyway**.

7. Enter your administrator password when prompted and follow the installer instructions to complete the installation. Once completed, the VST3 and AU plugins will be installed in the default system locations and can be opened in your favorite DAW by the name `ai-coustics Demo`.

- [Official Apple Documentation](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unknown-developer-mh40616/mac)
- AU Install Location: `Library/Audio/Plug-Ins/Components/ai-coustics Demo.component`
- VST3 Install Location: `/Library/Audio/Plug-Ins/VST3/ai-coustics Demo.vst3`

### Windows

To install the plugin on Windows, open the file `aic-sdk-plugin-windows.exe`. At the first time you will get a warning `Windows protected your PC`, click on `More info` and `Run anyway`. Now click `Yes` on the `User Account Control` dialog and follow the instructions of the installer. When it warns you that the folder `Common Files` already exists just proceed. Now you can open the plugin in your favorite DAW using the name `ai-coustics Demo`.

The plugin will be installed to `C:\Program Files\Common Files\VST3\`.

### Linux

Install the VST3 plugin to the default system location (`~/.vst3/`) by running the install script in your terminal:

```sh
./install.sh
```

The plugin can be opened in your favorite DAW by the name `ai-coustics Demo`.

## License Key

When you open the plugin in your DAW for the first time, a license key dialog will pop up. Paste the provided license key in the text input and activate the license. A license key can be acquired by getting in contact with us, and soon you will be able to create your own key in the developer portal.

The license keys are stored in the following files and can be placed there manually as well:

- Linux: `~/.config/aic/aic-sdk-license.txt`
- macOS: `~/Library/aic/aic-sdk-license.txt`
- Windows: `C:\Users\<user>\AppData\Roaming\aic\aic-sdk-license.txt`

---

Feel free to reach out to us if you encounter any issues or need further assistance.
