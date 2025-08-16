# Nimf Input Method Framework

[![Build Status](https://github.com/hamonikr/nimf/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/hamonikr/nimf/actions/workflows/build.yml)
[![Donate Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://ko.liberapay.com/hamonikr/)

![version](https://img.shields.io/badge/version-1.4.1-blue)
![toolkits](https://img.shields.io/badge/GTK-2%2F3%2F4-green)
![qt](https://img.shields.io/badge/Qt-5%2F6-green)
![arch](https://img.shields.io/badge/arch-x86__64%20%7C%20arm64-darkblue)
![ubuntu](https://img.shields.io/badge/Ubuntu-24.04%2B-orange)
![debian](https://img.shields.io/badge/Debian-Bookworm%2FTrixie-brown)
![fedora](https://img.shields.io/badge/Fedora-33%2B-blue)
![opensuse](https://img.shields.io/badge/openSUSE-Leap%2015.6-green)
![archlinux](https://img.shields.io/badge/Arch-Rolling-blue)

**🇺🇸 English** | [🇰🇷 한국어](README-ko.md)

---

Nimf is a lightweight, fast and extensible input method framework that supports Korean, Japanese, Chinese, and many other languages through multilingual input engines.

![nimf](docs/nimf.png)

## 📋 Table of Contents

- [About](#-about)
- [Key Features](#-key-features) 
- [Installation](#-installation)
- [Build from Source](#-build-from-source)
- [Contributing](#-contributing)

## 🎯 About

Nimf is maintained by the **HamoniKR Team** as a continuation of the original nimf project. This fork ensures the sustainability of this essential input method framework for Korean and multilingual text input on Linux systems.

### What's New
- **Full GTK4 support** with backwards compatibility (GTK2/3/4)
- **Modular packaging**: Core package + optional multilingual extension (`nimf-i18n`)
- **Enhanced platform support** across major Linux distributions
- **Improved Wayland compatibility** and environment integration

## ⚡ Key Features

### 🌐 **Multilingual Input Support**
- **Korean**: Advanced Hangul input with libhangul (Dubeolsik, Sebeolsik, etc.)
- **Japanese**: Comprehensive hiragana/katakana/kanji input via Anthy
- **Chinese**: Traditional and simplified Chinese with librime engine  
- **100+ Languages**: Extended multilingual support through m17n library

### 🖥️ **Universal Compatibility**
- **Toolkit Support**: GTK2, GTK3, GTK4, Qt5, Qt6
- **Display Protocols**: X11 and Wayland native support
- **Desktop Environments**: GNOME, KDE, XFCE, and others

### 🚀 **Performance & Usability**
- **Lightweight**: Minimal resource usage and fast startup
- **System Integration**: Native desktop indicator and configuration tools
- **Customizable**: Flexible keyboard layouts and input preferences

### 📦 **Supported Platforms**

| Architecture | Distributions |
|--------------|---------------|
| **x86_64** | Ubuntu 24.04+, Debian 12+, Fedora 33+, openSUSE 15.6+, Arch Linux |
| **arm64** | Ubuntu 24.04+, Debian 12+, Arch Linux |

## 🚀 Installation

### Automatic Installation (Recommended)

Install the latest version with a single command on all supported distributions:

```bash
# Using curl
curl -fsSL https://raw.githubusercontent.com/hamonikr/nimf/master/install | sudo bash

# Or using wget  
wget -qO- https://raw.githubusercontent.com/hamonikr/nimf/master/install | sudo -E bash -
```

**This script automatically:**
- ✅ Detects your distribution and installs the appropriate package
- ✅ **Handles ibus conflicts** (disables ibus-daemon if present)  
- ✅ Configures environment variables for all desktop environments
- ✅ Sets Nimf as the default input method

### Manual Installation

For advanced users who prefer manual installation, download the appropriate package from [Releases](https://github.com/hamonikr/nimf/releases) and install using your distribution's package manager.

**Examples:**
```bash
# Ubuntu/Debian
sudo dpkg -i nimf_*.deb && sudo apt-get install -f

# Fedora/RHEL  
sudo dnf install nimf-*.rpm

# Arch Linux
sudo pacman -U nimf-*.pkg.tar.zst

# openSUSE
sudo zypper install nimf-*.rpm
```

**Note**: Manual installation requires additional configuration steps. The automatic installation script above is recommended for most users.

## 🔧 Build from Source

### Quick Start

```bash
git clone --recurse-submodules https://github.com/hamonikr/nimf
cd nimf
./autogen.sh
./configure --prefix=/usr/local
make -j "$(nproc)"
sudo make install
```

### Docker-based Package Building (Recommended)

For cross-platform package building:

```bash
# Build for specific distributions
./scripts/build-docker.sh ubuntu.2404
./scripts/build-docker.sh debian.bookworm  
./scripts/build-docker.sh fedora.latest
./scripts/build-docker.sh arch

# Generated packages will be in dist/ folder
find dist/ -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*"
```

**📚 Detailed Documentation:**
- **Docker builds**: [DOCKER-BUILD.md](DOCKER-BUILD.md)
- **Source builds**: [BUILD.md](BUILD.md)
- **Platform guides**: [Wiki](https://github.com/hamonikr/nimf/wiki)

## 🤝 Contributing

### Quick Links
- **🐛 Report Issues**: [GitHub Issues](https://github.com/hamonikr/nimf/issues)
- **💬 Community**: [HamoniKR Community](https://hamonikr.org)  
- **🔗 Additional Resources**: [Project Wiki](https://github.com/hamonikr/nimf/wiki)

### How to Contribute

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### License

This project is licensed under the **GNU Lesser General Public License v3.0**.

See the [LICENSE](LICENSE) file for details.

---

**Maintained by [HamoniKR Team](https://hamonikr.org)** 💙  
*Ensuring sustainable multilingual input for the Linux ecosystem*

