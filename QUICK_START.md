# LinuxDroid Quick Start Guide

## 🚀 5-Minute Setup

### Step 1: Install the Package (1 minute)

```bash
# Download and install
sudo apt install ./linuxdroid_1.0.0_amd64.deb
```

### Step 2: Log Out and Back In (1 minute)

This is **required** for group membership changes to take effect.

```bash
# Or just reboot
sudo reboot
```

### Step 3: Launch LinuxDroid (3 minutes)

```bash
linuxdroid
```

Or find it in: **Applications → Development → LinuxDroid**

### Step 4: Follow the Setup Wizard

The wizard will guide you through:

1. ✅ **System Check** - Verifies your system is ready
2. ⚙️ **Configure Resources** - Set CPU cores and RAM
3. 📥 **Download Android** - Choose and download Android 9 (Recommended)
4. ⏳ **Wait for Download** - 1.2GB download (varies by connection speed)
5. 🎮 **Create Instance** - Name it and configure
6. ✨ **Done!** - Launch your Android emulator

## 📱 Using Your Emulator

### Start an Instance
1. Select instance from list
2. Click **Start**
3. Wait for Android to boot (2-3 minutes first time)

### Install Apps

**Method 1: Via ADB**
```bash
adb connect localhost:5555
adb install myapp.apk
```

**Method 2: Drag and Drop**
- Drag APK file into emulator window

**Method 3: Download in Android**
- Use Android browser to download APK
- Install from Downloads

## ⌨️ Essential Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Alt+F` | Fullscreen toggle |
| `Ctrl+Alt+G` | Release mouse |
| `Ctrl+Alt+Q` | Quit emulator |

## 🔧 Common Commands

```bash
# Check if KVM is working
kvm-ok

# Verify system requirements
cd LinuxDroid
./scripts/verify_system.sh

# Connect via ADB
adb connect localhost:5555

# View logs
journalctl -u linuxdroid-download.service -f

# Check running VMs
ps aux | grep qemu
```

## ❓ Quick Troubleshooting

### "KVM not available"
```bash
# Enable in BIOS, then:
sudo modprobe kvm
sudo modprobe kvm_intel  # or kvm_amd
```

### "Permission denied: /dev/kvm"
```bash
sudo usermod -aG kvm $USER
# Then log out and back in
```

### "Download failed"
- Wait for automatic retry (3 attempts)
- Check internet connection
- Use "Resume Download" button

### "Slow performance"
- Ensure KVM is enabled
- Allocate more CPU/RAM
- Use Android 9 (most optimized)

## 📚 Next Steps

- Read full [README.md](README.md) for detailed documentation
- Create multiple instances for different Android versions
- Configure ADB for app development
- Join our community for support

---

**Need help?** Report issues at: https://github.com/tripletech/linuxdroid/issues
