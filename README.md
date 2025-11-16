# LinuxDroid - Professional Android Emulator for Linux

<div align="center">

![LinuxDroid Logo](resources/icons/linuxdroid.svg)

**High-performance Android emulator for Ubuntu/Debian with KVM acceleration**

[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)]()
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)]()

</div>

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Building from Source](#building-from-source)
- [First Run Setup](#first-run-setup)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [License](#license)

## 🎯 Overview

LinuxDroid is a modern, production-ready Android emulator specifically designed for Ubuntu and Debian-based Linux distributions. It leverages QEMU/KVM for hardware acceleration, providing near-native performance for Android applications.

Unlike other emulators, LinuxDroid features:
- **Automated setup wizard** with guided Android image download
- **Background download service** with resume capability
- **Multi-instance support** for running multiple Android VMs
- **Native Linux integration** with system tray notifications
- **Professional packaging** as a .deb package for easy installation

## ✨ Features

### Core Features
- 🚀 **KVM Hardware Acceleration** - Near-native performance using KVM virtualization
- 📦 **Easy Installation** - Distributed as a .deb package with automated setup
- 🎨 **Modern Qt6 GUI** - Clean, intuitive user interface
- 🔄 **Multi-Instance Support** - Run multiple Android VMs simultaneously
- 📥 **Automatic Image Download** - Download Android x86 images directly from the wizard
- ⏸️ **Resume Downloads** - Interrupted downloads automatically resume
- 🔐 **SHA256 Verification** - Ensures downloaded images are authentic
- 🔧 **First-Run Wizard** - Six-screen setup process for easy configuration

### Technical Features
- **Background Download Service** - systemd service for managing downloads
- **ADB Bridge** - Connect via `adb connect localhost:5555`
- **Custom Configurations** - Per-instance CPU, RAM, and resolution settings
- **System Tray Integration** - Minimize to system tray
- **Graceful Error Handling** - Comprehensive error messages and recovery

## 💻 System Requirements

### Minimum Requirements
- **OS**: Ubuntu 20.04+ or Debian 11+
- **CPU**: 2 cores with VT-x/AMD-V support
- **RAM**: 8GB (4GB for VM + 4GB for host)
- **Disk**: 20GB free space
- **Graphics**: OpenGL 3.0+ compatible GPU

### Recommended Requirements
- **OS**: Ubuntu 22.04+ or Debian 12+
- **CPU**: 4+ cores with VT-x/AMD-V enabled
- **RAM**: 16GB (allows 8GB for VM)
- **Disk**: 50GB free space (for multiple instances)
- **Graphics**: Dedicated GPU with OpenGL 4.5+

### Required Software
- `qemu-system-x86` - QEMU virtualization
- `libvirt0` - Virtualization API
- `qt6-base-dev` - Qt6 libraries
- `libgl1` - OpenGL support
- `libsdl2-2.0-0` - SDL2 library
- `systemd` - System service manager

## 📦 Installation

### Quick Install (Recommended)

1. **Download the .deb package**:
   ```bash
   wget https://github.com/yourrepo/linuxdroid/releases/download/v1.0.0/linuxdroid_1.0.0_amd64.deb
   ```

2. **Install the package**:
   ```bash
   sudo apt install ./linuxdroid_1.0.0_amd64.deb
   ```

   This will automatically:
   - Install LinuxDroid and all dependencies
   - Add your user to `kvm` and `libvirt` groups
   - Set up directories in `/opt/linuxdroid`
   - Enable the background download service
   - Create desktop launcher

3. **Log out and log back in** (required for group membership changes)

4. **Launch LinuxDroid**:
   ```bash
   linuxdroid
   ```
   Or find it in your applications menu under "Development" → "LinuxDroid"

### Verifying Installation

Check that KVM is available:
```bash
kvm-ok
```

Expected output:
```
INFO: /dev/kvm exists
KVM acceleration can be used
```

If you see an error, enable virtualization in your BIOS/UEFI settings.

## 🔨 Building from Source

### Install Build Dependencies

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools \
                 qemu-system-x86 libvirt-dev libgl1-mesa-dev libsdl2-dev \
                 dpkg-dev debhelper
```

### Clone and Build

```bash
# Clone the repository
git clone https://github.com/yourrepo/linuxdroid.git
cd LinuxDroid

# Verify your system meets requirements
./scripts/verify_system.sh

# Build the .deb package
./scripts/build_deb.sh
```

The build script will:
1. ✅ Check all dependencies
2. 🏗️ Configure with CMake
3. ⚙️ Compile the application
4. 📦 Create the .deb package
5. 📋 Display package information

### Install Your Build

```bash
sudo apt install ./linuxdroid_1.0.0_amd64.deb
```

## 🚀 First Run Setup

When you launch LinuxDroid for the first time, you'll be guided through a 6-screen setup wizard:

### Screen 1: Welcome
- Introduction to LinuxDroid
- System requirements check
- Warnings about missing requirements

### Screen 2: System Configuration
- CPU core allocation
- RAM allocation
- KVM status verification
- Disk space check

### Screen 3: Android Image Selection
Choose from:
- **Android 9.0 (Pie)** - Recommended, 1.2GB
- **Android 11 (R)** - Modern features, 1.4GB
- **Android 13 (Tiramisu)** - Latest, 1.6GB

### Screen 4: Download Progress
- Real-time download progress
- Speed and ETA display
- Resume capability
- Background download option

### Screen 5: Instance Setup
- Instance name
- Display resolution (720p, 1080p, 1440p, 4K)
- CPU and RAM configuration
- Root access toggle

### Screen 6: Completion
- Success confirmation
- Quick tips and keyboard shortcuts
- Launch button

## 📖 Usage

### Creating New Instances

1. Click **"New Instance"** in the main window
2. Follow the setup wizard
3. Configure CPU, RAM, and resolution
4. Launch your instance

### Starting an Instance

1. Select an instance from the list
2. Click **"Start"**
3. The Android emulator window will open
4. Wait for Android to boot (first boot takes longer)

### Connecting via ADB

```bash
adb connect localhost:5555
adb devices
adb shell
```

### Installing APKs

```bash
adb install app.apk
```

Or drag and drop APK files into the emulator window.

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Alt+F` | Toggle fullscreen |
| `Ctrl+Alt+G` | Release mouse grab |
| `Ctrl+Alt+Q` | Quit emulator |
| `Ctrl+Alt+R` | Reboot Android |

### Managing Instances

- **Start**: Launch a stopped instance
- **Stop**: Gracefully shut down running instance
- **Delete**: Remove instance (confirmation required)
- **Settings**: Configure instance parameters

## 🔧 Troubleshooting

### KVM Not Available

**Symptom**: Warning that KVM is not available

**Solution**:
1. Check if virtualization is enabled:
   ```bash
   grep -E '(vmx|svm)' /proc/cpuinfo
   ```

2. If no output, enable VT-x (Intel) or AMD-V (AMD) in BIOS:
   - Reboot and enter BIOS/UEFI (usually F2, F10, or Del)
   - Look for "Virtualization Technology" or "SVM Mode"
   - Enable it and save

3. Check if KVM module is loaded:
   ```bash
   lsmod | grep kvm
   ```

4. If not loaded, load it:
   ```bash
   sudo modprobe kvm
   sudo modprobe kvm_intel  # For Intel
   # OR
   sudo modprobe kvm_amd    # For AMD
   ```

### Download Failed

**Symptom**: Android image download fails

**Solution**:
- Check internet connection
- Downloads automatically retry 3 times
- Use "Resume Download" if interrupted
- Check `/var/log/linuxdroid/download.log` for details

### Permission Denied: /dev/kvm

**Symptom**: Cannot access /dev/kvm

**Solution**:
```bash
# Verify you're in kvm group
groups

# If not, add yourself
sudo usermod -aG kvm $USER

# Log out and log back in
```

### Instance Won't Start

**Symptom**: Error when starting instance

**Solutions**:
1. Check if Android image exists:
   ```bash
   ls -lh /opt/linuxdroid/images/
   ```

2. Verify QEMU is installed:
   ```bash
   which qemu-system-x86_64
   ```

3. Check instance configuration:
   ```bash
   cat /opt/linuxdroid/instances/*/config.json
   ```

4. Check logs:
   ```bash
   journalctl -u linuxdroid-download.service
   ```

### Low Performance

**Solutions**:
- Ensure KVM is enabled (hardware acceleration)
- Allocate more CPU cores and RAM
- Close other resource-intensive applications
- Use a lower resolution (720p instead of 4K)
- Update graphics drivers

### Black Screen on Boot

**Solution**:
- Wait 2-3 minutes (first boot is slow)
- Increase RAM allocation
- Try a different Android version (Android 9 is most stable)

## 🛠️ Development

### Project Structure

```
LinuxDroid/
├── src/
│   ├── core/           # QEMU manager, VM config, downloads
│   ├── gui/            # Qt6 GUI components
│   ├── utils/          # System checker, helpers
│   ├── main.cpp        # Application entry point
│   └── daemon.cpp      # Background download service
├── debian/             # Debian package files
├── resources/          # Icons, JSON data
├── scripts/            # Build and verification scripts
├── CMakeLists.txt      # CMake build configuration
└── README.md
```

### Building for Development

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
./linuxdroid
```

### Running Tests

```bash
# System verification
./scripts/verify_system.sh

# Manual daemon test
./build/linuxdroid-daemon <url> <destination>
```

### Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin feature-name`
5. Submit a pull request

## 📝 Configuration Files

### Android Images JSON
Located at `/opt/linuxdroid/android_images.json`

Contains download URLs, checksums, and metadata for Android x86 images.

### Instance Config
Located at `/opt/linuxdroid/instances/<name>/config.json`

Example:
```json
{
  "name": "My Android",
  "imagePath": "/opt/linuxdroid/images/android-9-pie.iso",
  "cpuCores": 4,
  "ramMB": 4096,
  "resolutionWidth": 1920,
  "resolutionHeight": 1080,
  "rootEnabled": false
}
```

## 🔒 Security

- Downloads are verified using SHA256 checksums
- Systemd service runs with minimal privileges
- No telemetry or data collection
- Open source for transparency

## 🐛 Known Issues

1. **First boot slow** - Android initialization takes 2-3 minutes
2. **Camera not working** - Webcam passthrough not yet implemented
3. **Audio glitches** - Occasional audio crackling (configure PulseAudio)
4. **Google Play** - May require manual installation via OpenGApps

## 📚 Resources

- **Android-x86 Project**: https://www.android-x86.org/
- **QEMU Documentation**: https://www.qemu.org/docs/master/
- **KVM Documentation**: https://www.linux-kvm.org/

## 🙏 Acknowledgments

- Android-x86 project for x86 Android builds
- QEMU/KVM teams for virtualization technology
- Qt framework for the GUI

## 📄 License

LinuxDroid is licensed under the GNU General Public License v3.0.

See [LICENSE](LICENSE) for full details.

```
Copyright (C) 2024 Dharun Ashokkumar <contact@tripletech.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

## 📧 Contact

- **Author**: Dharun Ashokkumar
- **Email**: contact@tripletech.com
- **Website**: https://tripletech.com
- **GitHub**: https://github.com/tripletech/linuxdroid

---

<div align="center">

**Made with ❤️ for the Linux community**

[Report Bug](https://github.com/tripletech/linuxdroid/issues) ·
[Request Feature](https://github.com/tripletech/linuxdroid/issues) ·
[Documentation](https://github.com/tripletech/linuxdroid/wiki)

</div>
