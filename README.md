# nimf

[![Build Status](https://github.com/hamonikr/nimf/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/hamonikr/nimf/actions/workflows/build.yml)
[![Donate Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://ko.liberapay.com/hamonikr/)

![version](https://img.shields.io/badge/version-1.3.9-blue)
![toolkits](https://img.shields.io/badge/GTK-2%2F3%2F4-green)
![qt](https://img.shields.io/badge/Qt-5%2F6-green)
![arch](https://img.shields.io/badge/arch-x86__64%20%7C%20arm64-darkblue)
![ubuntu](https://img.shields.io/badge/Ubuntu-22.04%2B-orange)
![debian](https://img.shields.io/badge/Debian-12%2B-brown)
![fedora](https://img.shields.io/badge/Fedora-33%2B-blue)
![opensuse](https://img.shields.io/badge/openSUSE-Leap%2015.6-green)
![archlinux](https://img.shields.io/badge/Arch-Rolling-blue)

[English](#nimf) | [한국어](#가볍고-빠른-입력기-프레임워크-nimf)

## Table of Contents

- [Nimf](#nimf)
  - [What’s New in 1.3.9](#whats-new-in-139)
  - [Features](#features)
  - [Supported Platforms](#supported-platforms)
  - [Installation](#installation)
    - [Ubuntu/Debian/LinuxMint](#ubuntudebianlinuxmint)
    - [Arch Linux, Manjaro](#arch-linux-manjaro)
    - [Fedora, CentOS, RedHat](#fedora-centos-redhat)
    - [openSUSE Leap 15.6](#opensuse-leap-156)
  - [Configuration](#configuration)
  - [Build Instructions](#build-instructions)
  - [Add Custom Hangul Keyboard Layouts](#add-custom-hangul-keyboard-layouts)
  - [Additional Resources](#additional-resources)
  - [License](#license)
  - [Issue](#issue)
  - [Contribution](#contribution)
- [Korean Section](#가볍고-빠른-입력기-프레임워크-nimf)

Nimf is a lightweight, fast and extensible input method framework.

![nimf](docs/nimf.png)

## What’s New in 1.3.9

- Full GTK4 support (GTK2/GTK3/GTK4 can co-exist)
- Debian packaging split into two packages:
  - `nimf`: core framework + Korean(libhangul), Japanese(anthy), Chinese(rime)
  - `nimf-i18n`: additional multilingual engines (m17n)
- Updated build options:
  - `--enable-gtk4` to build GTK4 IM module
  - `--with-gtk=3|4` to select GTK version for `nimf-settings`
  - Engine toggles: `--disable-nimf-{libhangul,anthy,m17n,rime}`
  - `--disable-x11` for Wayland-only setups
  - `--with-im-config-data`, `--with-imsettings-data` for integration

## Features

- Input Method Server: `nimf`
- Language Engines:
  - System Keyboard
  - Korean (based on libhangul)
  - Japanese (based on anthy)
  - Chinese (based on librime)
  - Various languages (based on m17n, via `nimf-i18n`)
- Service Modules:
  - Indicator (ayatana-appindicator)
  - Wayland, XIM (based on IMdkit)
  - Preedit window, Candidate window
- Client Modules:
  - GTK+2, GTK+3, GTK4, Qt5, Qt6

## Supported Platforms

- x86_64
  - Ubuntu (>= 18.04; GTK4 support on >= 22.04)
  - Debian (>= 10; GTK4 support on >= 12)
  - Arch Linux (Rolling)
  - Fedora (>= 33)
  - openSUSE Leap (15.6)
- arm64
  - Ubuntu (>= 18.04)
  - Debian (>= 10)
  - Arch Linux (Rolling)

## Installation

### Ubuntu/Debian/LinuxMint

On Ubuntu 21.10 or later, `ibus-daemon` may auto-start and conflict with Nimf.

Method 1: Remove ibus

```bash
sudo apt purge ibus
```

Method 2: Disable ibus-daemon

```bash
sudo mv /usr/bin/ibus-daemon /usr/bin/ibus-daemon.bak
```

Install Nimf

```bash
curl -fsSL https://repo.hamonikr.org/install | sudo bash

sudo apt install nimf

im-config -n nimf
```

Additional multilingual engines

```bash
sudo apt install nimf-i18n
```

### Arch Linux, Manjaro

Option A) Build from source with makepkg

```bash
# Install latest libhangul-git
git clone https://aur.archlinux.org/libhangul-git.git
cd libhangul-git
makepkg -si

# Build nimf package
git clone https://github.com/hamonikr/nimf.git
cd nimf
makepkg -si
```

Option B) Install from Releases

```bash
# Download latest release package from the Releases page and install
sudo pacman -U ./nimf-*.pkg.tar.zst
```

### Fedora, CentOS, RedHat

Install from Releases (RPM)

```bash
# Download the latest .rpm from the Releases page and install
sudo dnf install ./nimf-*.rpm
```

### openSUSE Leap 15.6

1. Install package GPG key

```bash
wget https://github.com/hamonikr/nimf/raw/master/RPM-GPG-KEY-nimf
sudo rpm --import RPM-GPG-KEY-nimf
rpm -qa gpg-pubkey --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" | grep e42665b8
```

1. Install package

```bash
# Download the latest .rpm from the Releases page
sudo zypper install ./nimf-*.rpm
```

1. Reboot

## Configuration

Set IM modules in your shell profile if needed:

```bash
export GTK_IM_MODULE=nimf
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"

# For GTK4 applications (optional)
export GTK4_IM_MODULE=nimf
```

## Build Instructions

See [BUILD.md](BUILD.md) for full, distro-specific packaging guides.

Quick start (from source):

```bash
git clone --recurse-submodules https://github.com/hamonikr/nimf
cd nimf
./autogen.sh
./configure --prefix=/usr/local
make -j "$(nproc)"
sudo make install
```

Key configure options:

- `--enable-gtk4`: Build GTK4 IM module (auto-detect by default)
- `--with-gtk=3|4`: Select GTK version for `nimf-settings` (default: 3)
- `--disable-nimf-libhangul`, `--disable-nimf-anthy`, `--disable-nimf-m17n`, `--disable-nimf-rime`
- `--disable-x11`: Build without X11 (Wayland-only)
- `--with-im-config-data`, `--with-imsettings-data`: Install integration data

## Add Custom Hangul Keyboard Layouts

Hangul layouts for libhangul are defined as XML files in the following paths:
- System-wide: `/usr/share/libhangul/keyboards`
- Per-user: `$HOME/.local/share/libhangul/keyboards` or `$XDG_DATA_HOME/libhangul/keyboards`

Refer to the default layouts and combination file for structure:

- `docs/hangul-keyboard-2.xml`
- `docs/hangul-combination-default.xml`

General structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<hangul-keyboard id="2" type="jamo">
  <name>Dubeolsik</name>
  <name xml:lang="ko">두벌식</name>

  <map id="0">
    <item key="0x41" value="0x1106"/>
    <item key="0x42" value="0x1172"/>
    ...
  </map>

  <combination id="0">
    <item first="0x1100" second="0x1100" result="0x1101"/>
    <item first="0x1169" second="0x1161" result="0x116a"/>
    ...
  </combination>
  <!-- or -->
  <include file="hangul-combination-default.xml"/>
</hangul-keyboard>
```

Element notes:

- `hangul-keyboard`: Root layout element
  - `id`: ID used by the input engine
  - `type`: `jamo`, `jamo-yet`, `jaso`, `jaso-yet`, `ro`
- `name`: Visible name; localized via `xml:lang`
- `map`: Key-to-Unicode mapping (ID fixed to 0)
- `combination`: Sequential composition rules (ID fixed to 0)
- `include`: Inline another XML by relative/absolute path

## Additional Resources

- Manjaro: <https://github.com/hamonikr/nimf/wiki/Manjaro-build>
- CentOS 8: <https://blog.naver.com/dfnk5516/222074913406>
- Raspberry Pi 4 arm64: <https://github.com/hamonikr/nimf/wiki/Install-nimf-on-raspberry-pi-4---arm64>
- Armbian: <https://github.com/hamonikr/nimf/wiki/Armbian-build>
- Manjaro ARM: <https://github.com/hamonikr/nimf/wiki/Manjaro-build>
- Arch AUR: <https://aur.archlinux.org/packages/nimf-git/>
- Others: <https://github.com/hamonikr/nimf/wiki/How-to-Build-and-Install-with-Others-Distro>

---

## 가볍고 빠른 입력기 프레임워크 nimf

[Go to English](#nimf)

이 프로젝트는 한글입력기 nimf 가 더이상 [지속되기 힘든 상황](https://launchpad.net/~hodong/+archive/ubuntu/nimf) 이 되었기 때문에

프로젝트의 지속적인 사용을 위해 [nimf Project](https://gitlab.com/nimf-i18n/nimf)를 포크한 프로젝트입니다.

하모니카 개발팀은 개방형 OS 배포에 필수적인 한글 입력기에 대한 관리가 필요하다고 생각하며 앞으로 하모니카 팀에서 직접 nimf 프로젝트를 계속 관리합니다.

## 1.3.9의 주요 변경사항
- GTK4 완전 지원 (GTK2/GTK3/GTK4 동시 환경 지원)
- 데비안 패키지 분리:
  - `nimf`: 코어 + 한글(libhangul), 일본어(anthy), 중국어(rime)
  - `nimf-i18n`: 다국어 엔진(m17n) 추가 패키지
- 빌드 옵션 업데이트:
  - `--enable-gtk4`: GTK4 IM 모듈 빌드
  - `--with-gtk=3|4`: `nimf-settings`의 GTK 버전 선택
  - 엔진 토글: `--disable-nimf-{libhangul,anthy,m17n,rime}`
  - `--disable-x11`: Wayland 전용 빌드
  - 통합 옵션: `--with-im-config-data`, `--with-imsettings-data`

## 기능
- 입력기 서버: `nimf`
- 언어 엔진:
  - 시스템 키보드
  - 한글 (libhangul)
  - 일본어 (anthy)
  - 중국어 (librime)
  - 다양한 언어 (m17n, `nimf-i18n` 설치 시)
- 서비스 모듈:
  - 인디케이터 (ayatana-appindicator)
  - Wayland, XIM (IMdkit 기반)
  - 미리보기/후보창
- 클라이언트 모듈:
  - GTK+2, GTK+3, GTK4, Qt5, Qt6

## 지원 플랫폼
- x86_64
  - Ubuntu (>= 18.04; GTK4는 >= 22.04)
  - Debian (>= 10; GTK4는 >= 12)
  - Arch Linux (Rolling)
  - Fedora (>= 33)
  - openSUSE Leap (15.6)
- arm64
  - Ubuntu (>= 18.04)
  - Debian (>= 10)
  - Arch Linux (Rolling)

## 설치

### Ubuntu(amd64, arm64), Debian(amd64, arm64), LinuxMint
Ubuntu 21.10 이상에서는 `ibus-daemon`이 자동 시작되어 nimf와 충돌할 수 있습니다.

방법 1: ibus 제거
```bash
sudo apt purge ibus
```
방법 2: ibus-daemon 비활성화
```bash
sudo mv /usr/bin/ibus-daemon /usr/bin/ibus-daemon.bak
```

Nimf 설치
```bash
curl -fsSL https://repo.hamonikr.org/install | sudo bash

sudo apt install nimf

im-config -n nimf
```

다국어 엔진 추가(m17n)
```bash
sudo apt install nimf-i18n
```

### Arch Linux, Manjaro

방법 A) makepkg로 빌드 설치
```bash
# 최신 libhangul-git 설치
git clone https://aur.archlinux.org/libhangul-git.git
cd libhangul-git
makepkg -si

# nimf 패키지 빌드
git clone https://github.com/hamonikr/nimf.git
cd nimf
makepkg -si
```

방법 B) 릴리스 패키지 설치

```bash
# 릴리스 페이지에서 최신 패키지 다운로드 후 설치
sudo pacman -U ./nimf-*.pkg.tar.zst
```

### Fedora, CentOS, RedHat

릴리스(RPM)에서 설치

```bash
# 릴리스 페이지에서 최신 .rpm 다운로드 후 설치
sudo dnf install ./nimf-*.rpm
```

### openSUSE Leap 15.6

1) nimf 저장소 GPG 키 설치

```bash
wget https://github.com/hamonikr/nimf/raw/master/RPM-GPG-KEY-nimf
sudo rpm --import RPM-GPG-KEY-nimf
rpm -qa gpg-pubkey --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" | grep e42665b8
```

2) 패키지 설치

```bash
# 릴리스 페이지에서 최신 .rpm 다운로드 후 설치
sudo zypper install ./nimf-*.rpm
```

3) 재부팅

#### 설정

필요 시 셸 프로파일에 입력기 모듈 설정을 추가합니다.

```bash
export GTK_IM_MODULE=nimf
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"

# GTK4 앱을 사용하는 경우(선택)
export GTK4_IM_MODULE=nimf
```

## 빌드 안내
배포판별 패키지 생성 방법은 [BUILD.md](BUILD.md)를 참고하세요.

소스에서 빌드(요약):
```bash
git clone --recurse-submodules https://github.com/hamonikr/nimf
cd nimf
./autogen.sh
./configure --prefix=/usr/local
make -j "$(nproc)"
sudo make install
```

주요 설정 옵션:
- `--enable-gtk4`: GTK4 IM 모듈 빌드(기본 자동 감지)
- `--with-gtk=3|4`: `nimf-settings`의 GTK 버전 선택(기본 3)
- `--disable-nimf-libhangul`, `--disable-nimf-anthy`, `--disable-nimf-m17n`, `--disable-nimf-rime`
- `--disable-x11`: X11 제외(Wayland 전용)
- `--with-im-config-data`, `--with-imsettings-data`: 통합 데이터 설치

## 한글 자판 배열 추가
libhangul에서 사용하는 한글 자판 배열은 다음 경로에 XML 파일로 정의됩니다.
- 시스템 전체: `/usr/share/libhangul/keyboards`
- 사용자별: `$HOME/.local/share/libhangul/keyboards` 또는 `$XDG_DATA_HOME/libhangul/keyboards`

기본 제공 예제를 참고하세요.
- `docs/hangul-keyboard-2.xml`
- `docs/hangul-combination-default.xml`

일반적인 구조:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<hangul-keyboard id="2" type="jamo">
  <name>Dubeolsik</name>
  <name xml:lang="ko">두벌식</name>

  <map id="0">
    <item key="0x41" value="0x1106"/>
    <item key="0x42" value="0x1172"/>
    ...
  </map>

  <combination id="0">
    <item first="0x1100" second="0x1100" result="0x1101"/>
    <item first="0x1169" second="0x1161" result="0x116a"/>
    ...
  </combination>
  <!-- 또는 -->
  <include file="hangul-combination-default.xml"/>
</hangul-keyboard>
```

요소 설명:
- `hangul-keyboard`: 루트 요소
  - `id`: 입력기에서 사용하는 식별자
  - `type`: `jamo`, `jamo-yet`, `jaso`, `jaso-yet`, `ro`
- `name`: 표시 이름, `xml:lang`로 지역화 가능
- `map`: 키-유니코드 매핑(항상 id=0)
- `combination`: 연속 조합 규칙(항상 id=0)
- `include`: 다른 XML을 경로로 인라인 포함

## LICENSE

- GNU Lesser General Public License v3.0 ([한글 해석](https://olis.or.kr/license/Detailselect.do?lId=1073))

Nimf is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nimf is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program; If not, see <http://www.gnu.org/licenses/>.

## ISSUE

사용 중 이슈는 깃허브 이슈를 이용하시거나 [하모니카 커뮤니티](https://hamonikr.org)를 방문해서 알려주세요.

For any issues you encounter while using this software, please use GitHub Issues or visit the [Hamonikr Community](https://hamonikr.org).

## Contribution

깃허브 저장소를 포크하신 후 수정하실 내용을 수정하고 PR을 요청하시면 하모니카 팀에서 리뷰 후 반영됩니다.

To contribute, please fork the GitHub repository, make your changes, and submit a PR. The Hamonikr team will review and integrate your contributions.

