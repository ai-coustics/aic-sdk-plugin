# GitHub Actions Setup Guide

This guide explains how to set up and use the GitHub Actions workflows for the ai-coustics SDK Plugin project.

## Overview

The project includes several GitHub Actions workflows that automate building, testing, and releasing the plugin across multiple platforms:

1. **Build and Release** - Creates official releases with installers
2. **Test Build** - Tests builds on pull requests
3. **Manual Build** - Flexible manual builds for development
4. **Test Installers** - Validates installer scripts

## Initial Setup

### 1. Repository Configuration

Ensure your repository has the following settings:

#### Actions Permissions
- Go to **Settings** → **Actions** → **General**
- Set **Actions permissions** to "Allow all actions and reusable workflows"
- Set **Workflow permissions** to "Read and write permissions"
- Check "Allow GitHub Actions to create and approve pull requests"

#### Branch Protection (Recommended)
- Go to **Settings** → **Branches**
- Add protection rule for `main` branch:
  - Require status checks to pass before merging
  - Require branches to be up to date before merging
  - Include "Test Build" workflow checks

### 2. Workflow Files

The following workflow files should be in `.github/workflows/`:

- `build-and-release.yml` - Main release workflow
- `test-build.yml` - PR testing workflow  
- `manual-build.yml` - Manual development builds
- `test-installers.yml` - Installer validation

## How to Use

### Creating Releases

#### Automatic Release (Recommended)
1. Ensure all changes are merged to `main`
2. Update version in `CMakeLists.txt` if needed:
   ```cmake
   project(AicSdkPlugin VERSION 0.6.2 LANGUAGES C CXX)
   ```
3. Create and push a version tag:
   ```bash
   git tag v0.6.2
   git push origin v0.6.2
   ```
4. The workflow automatically:
   - Builds for Linux, macOS, Windows
   - Creates installer packages
   - Creates GitHub release with artifacts

#### Manual Release
1. Go to **Actions** → **Build and Release**
2. Click **Run workflow**
3. Select branch (usually `main`)
4. Click **Run workflow**

### Development Testing

#### Pull Request Testing
- Automatically runs when PRs are created/updated
- Tests build on all platforms
- Validates installer scripts
- No artifacts created

#### Manual Development Builds
1. Go to **Actions** → **Manual Build**
2. Click **Run workflow**
3. Configure options:
   - **Build type**: Release, Debug, or RelWithDebInfo
   - **Platforms**: All, or specific platform
   - **Create installers**: Enable to generate packages
4. Download artifacts from completed workflow

#### Testing Installer Scripts
1. Go to **Actions** → **Test Installers**
2. Runs automatically when installer scripts change
3. Can be run manually with installation testing enabled

## Workflow Details

### Build Artifacts

Each platform creates specific artifacts:

#### Linux
- **Format**: `.tar.gz`
- **Contents**: VST3 plugin + install script
- **Install**: Extract and run `./install.sh`

#### macOS
- **Format**: `.tar.gz`
- **Contents**: VST3/AU/Standalone + multiple installers
- **Install**: Double-click `Install ai-coustics Plugins.app`

#### Windows
- **Format**: `.zip`
- **Contents**: VST3 plugin + multiple installers
- **Install**: Double-click `Install ai-coustics Plugins.exe`

### Build Environment

#### Linux (Ubuntu 22.04)
```bash
# Dependencies installed automatically
sudo apt-get install libasound2-dev libjack-jackd2-dev libx11-dev ...
```

#### macOS (macOS 13)
```bash
# Universal binaries (Intel + Apple Silicon)
-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.11
```

#### Windows (Windows Server 2022)
```powershell
# Visual Studio 2022, PS2EXE for installer creation
cmake -G "Visual Studio 17 2022" -A x64
```

## Customization

### Adding New Platforms

1. Add new job to workflow files:
   ```yaml
   build-newplatform:
     runs-on: [runner-image]
     steps:
       - name: Checkout code
         uses: actions/checkout@v4
       # Add platform-specific steps
   ```

2. Update dependency arrays:
   ```yaml
   create-release:
     needs: [build-linux, build-macos, build-windows, build-newplatform]
   ```

### Modifying Build Configuration

Edit environment variables:
```yaml
env:
  CMAKE_BUILD_TYPE: Release
  JUCE_BUILD_PARALLEL_JOBS: 4
  # Add custom variables
```

### Changing Retention Policies

Modify artifact upload steps:
```yaml
- uses: actions/upload-artifact@v4
  with:
    retention-days: 30  # Change from default 7
```

## Troubleshooting

### Common Build Failures

#### JUCE Dependencies (Linux)
```
Error: Could not find X11 libraries
```
**Solution**: Ensure all Linux dependencies are installed in workflow

#### CMake Configuration
```
Error: Could not find CMAKE_CXX_COMPILER
```
**Solution**: Verify compiler installation and CMake generator settings

#### SDK Download Failures
```
Error: Failed to fetch ai-coustics SDK
```
**Solution**: Check network connectivity and SDK repository access

### Release Issues

#### Permission Denied
```
Error: Resource not accessible by integration
```
**Solution**: Check repository workflow permissions and GITHUB_TOKEN scope

#### Tag Format Issues
```
Release not triggered
```
**Solution**: Ensure tags follow `v*` pattern (e.g., `v1.0.0`)

#### Artifact Upload Failures
```
Error: Artifact upload failed
```
**Solution**: Check artifact size limits and file permissions

### Platform-Specific Issues

#### macOS Code Signing
```
Warning: Code signature not found
```
**Solution**: Add code signing step for distribution:
```yaml
- name: Sign macOS artifacts
  run: |
    codesign --sign "Developer ID" "path/to/plugin"
```

#### Windows Execution Policy
```
Error: PS2EXE installation failed
```
**Solution**: PowerShell execution policy handled automatically in workflow

#### Linux Missing Libraries
```
Error: library not found at runtime
```
**Solution**: Ensure static linking or document runtime dependencies

## Best Practices

### Version Management
1. Keep version in `CMakeLists.txt` as single source of truth
2. Use semantic versioning (e.g., `v1.2.3`)
3. Document breaking changes in release notes

### Testing Strategy
1. Test all changes in PR builds before merging
2. Use manual builds for experimental features
3. Validate installer scripts after modifications

### Security Considerations
1. Never commit secrets or API keys
2. Use repository secrets for sensitive data
3. Regularly update action versions
4. Review workflow permissions periodically

### Performance Optimization
1. Use parallel builds (`-j` flag)
2. Cache dependencies when possible
3. Limit artifact retention time
4. Use matrix builds for multiple configurations

## Monitoring

### Workflow Status
- Monitor workflows in **Actions** tab
- Set up notifications for failed builds
- Check artifact download statistics

### Build Analytics
- Review build times across platforms
- Monitor artifact sizes
- Track download counts for releases

## Support

### Getting Help
- Check workflow logs for detailed error messages
- Review this documentation and README files
- Create issues in the repository for workflow problems

### Contributing
- Test workflow changes in forks first
- Update documentation when modifying workflows
- Follow GitHub Actions best practices