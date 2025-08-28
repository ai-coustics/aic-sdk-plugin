# GitHub Workflows

This directory contains GitHub Actions workflows for building and releasing the ai-coustics SDK Plugin.

## Workflows

### 1. `build-and-release.yml`
**Purpose**: Creates official releases with installers for all platforms.

**Triggers**:
- When a tag starting with `v` is pushed (e.g., `v1.0.0`)
- Manual dispatch via GitHub Actions UI
- Pull requests (build only, no release)

**What it does**:
- Builds the plugin for Linux, macOS, and Windows
- Creates platform-specific installer packages
- Uploads artifacts to the GitHub release (only for tags)

**Artifacts created**:
- `ai-coustics-demo-plugin-linux.tar.gz` - Linux VST3 + install script
- `ai-coustics-demo-plugin-macos.tar.gz` - macOS VST3/AU/Standalone + installers
- `ai-coustics-demo-plugin-windows.zip` - Windows VST3 + installers

### 2. `test-build.yml`
**Purpose**: Tests builds on pull requests without creating releases.

**Triggers**:
- Pull requests to `main` or `develop` branches
- Manual dispatch via GitHub Actions UI

**What it does**:
- Builds on all platforms to verify compatibility
- Tests installer script syntax
- Checks version consistency
- Validates file structure

### 3. `manual-build.yml`
**Purpose**: Provides flexible manual builds for testing and development.

**Triggers**:
- Manual dispatch only

**Options**:
- **Build Type**: Release, Debug, or RelWithDebInfo
- **Platforms**: All, Linux only, macOS only, or Windows only
- **Create Installers**: Whether to generate installer packages

## How to Create a Release

### Automatic Release (Recommended)
1. Ensure your changes are merged to the main branch
2. Create and push a version tag:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
3. The workflow will automatically:
   - Build for all platforms
   - Create installer packages
   - Create a GitHub release with download links

### Manual Release
1. Go to the "Actions" tab in GitHub
2. Select "Build and Release" workflow
3. Click "Run workflow"
4. Choose your options and run

## Development Workflow

### Testing Changes
1. Create a pull request
2. The `test-build.yml` workflow will automatically run
3. Check that all builds pass before merging

### Manual Testing
1. Go to "Actions" → "Manual Build"
2. Select your desired options:
   - Choose build type (usually Release for testing)
   - Select specific platform if needed
   - Enable installer creation to test packages
3. Download artifacts from the completed workflow

## Build Requirements

### Linux (Ubuntu 22.04)
- Standard JUCE dependencies (ALSA, JACK, X11, etc.)
- CMake 3.14+
- GCC or Clang with C++17 support

### macOS (macOS 13)
- Xcode command line tools
- CMake 3.14+
- Supports both Intel (x86_64) and Apple Silicon (arm64)

### Windows (Windows Server 2022)
- Visual Studio 2022 Build Tools
- CMake 3.14+
- PowerShell 5.0+ (for installer creation)

## Workflow Configuration

### Environment Variables
- `CMAKE_BUILD_TYPE`: Release (default), Debug, or RelWithDebInfo
- `JUCE_BUILD_PARALLEL_JOBS`: Number of parallel build jobs (default: 4)

### Customizing Builds
Edit the workflow files to:
- Add new platforms or architectures
- Modify build configurations
- Add additional testing steps
- Change artifact retention policies

### Secrets Required
- `GITHUB_TOKEN`: Automatically provided by GitHub Actions
- No additional secrets needed for basic functionality

## Troubleshooting

### Build Failures
1. Check the workflow logs for specific error messages
2. Common issues:
   - Missing dependencies on Linux
   - CMake configuration errors
   - JUCE library fetch failures

### Release Issues
1. Ensure you have write permissions to the repository
2. Check that the tag follows the `v*` pattern (e.g., `v1.0.0`)
3. Verify all build jobs completed successfully

### Installer Creation Issues
1. **macOS**: App installer creation may fail if scripts aren't executable
2. **Windows**: PS2EXE installation requires PowerShell execution policies
3. **Linux**: Archive creation depends on proper file permissions

### Platform-Specific Notes

#### macOS
- Universal binaries (Intel + Apple Silicon) are created automatically
- App installer provides the best user experience
- Code signing not included (add separately for distribution)

#### Windows
- Executable installer (.exe) provides the best user experience
- PowerShell scripts may require execution policy changes on user machines
- Batch files (.bat) work universally without policy changes

#### Linux
- Single architecture (x64) builds
- Simple tar.gz archives with shell scripts
- Tested on Ubuntu; should work on most distributions

## Contributing

When modifying workflows:
1. Test changes in a fork first
2. Use the manual build workflow to validate
3. Update this README if adding new workflows or options
4. Follow GitHub Actions best practices for security and performance