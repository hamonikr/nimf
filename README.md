[![Build Status](https://github.com/hamonikr/nimf/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/hamonikr/nimf/actions/workflows/build.yml)
[![Donate Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://ko.liberapay.com/hamonikr/)

![x86_64](https://img.shields.io/badge/amd64-darkblue)
![ubuntu](https://img.shields.io/badge/ubuntu->=18.04-red)
![debian](https://img.shields.io/badge/debian->=10-brown)
![arch](https://img.shields.io/badge/archlinux-rolling-blue)
![manjaro](https://img.shields.io/badge/manjaro-x86_64-freen)
![fedora-33](https://img.shields.io/badge/fedora->=33-blue)
![opensuse](https://img.shields.io/badge/opensuse-leap-green)

![arm64](https://img.shields.io/badge/arm64-purple)
![ubuntu](https://img.shields.io/badge/ubuntu->=18.04-red)
![debian](https://img.shields.io/badge/debian->=10-brown)
![arch](https://img.shields.io/badge/archlinux-rolling-blue)
![manjaro](https://img.shields.io/badge/manjaro-x86_64-freen)
![fedora-33](https://img.shields.io/badge/fedora->=33-blue)


[English](#nimf) | [한국어](#가볍고-빠른-입력기-프레임워크-nimf)

# nimf

Nimf is a lightweight, fast and extensible input method framework.

![nimf](docs/nimf.png)

Nimf provides:
  * Input Method Server:
    * nimf

  * Language Engines:
    * System keyboard
    * Chinese (based on librime)
    * Japanese (based on anthy)
    * Korean (based on libhangul)
    * Various languages (based on m17n)

  * Service Modules:
    * Indicator (based on appindicator)
    * Wayland
    * NIM (Nimf Input Method)
    * XIM (based on IMdkit)
    * Preedit window
    * Candidate

  * Client Modules:
    * GTK+2, GTK+3, Qt5, Qt6
    
  * Settings tool to configure the Nimf:
    * nimf-settings

# Install

## Ubuntu(amd64, arm64), Debian(amd64, arm64), LinuxMint
On Ubuntu 21.10 or later distributions, the ibus-daemon starts automatically, causing conflicts with the input method nimf. 

Method 1: Remove ibus
```
sudo apt purge ibus
```
Method 2: Disable ibus-daemon
```
sudo mv /usr/bin/ibus-daemon /usr/bin/ibus-daemon.bak
```

Install nimf
```
wget -qO- https://pkg.hamonikr.org/add-hamonikr.apt | sudo -E bash -

sudo apt install nimf nimf-libhangul

im-config -n nimf
```

If you want to use other languages (Japanese, Chinese, etc.)
```
sudo apt install libnimf1 nimf nimf-anthy nimf-dev nimf-libhangul nimf-m17n nimf-rime
```

## Arch Linux, Manjaro

 1) Download and Install
```
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-1-any.pkg.tar.zst

sudo pacman -U ./nimf-1.3.8-1-any.pkg.tar.zst
``` 

 2) im setting
```
vi ~/.xprofile

export GTK_IM_MODULE=nimf
export QT4_IM_MODULE="nimf"
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"
```

## fedora, centos, redhat

 1) Download and Install
```
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-1.fc40.x86_64.rpm

sudo yum install ./nimf-1.3.8-1.fc40.x86_64.rpm
``` 

## opensuse-leap

 1) Download and Install
```
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.x86_64.rpm

sudo rpm -ivh ./nimf-1.3.8-2.opensuse_leap.x86_64.rpm

# If you want to install only Korean
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

sudo rpm -ivh ./nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

``` 

## Build Instructions

For detailed build instructions, see the following sections in the [BUILD.md](BUILD.md) file:

- [Building Debian Package](BUILD.md#debian-package)
- [Building Arch Linux Package](BUILD.md#arch-linux-package)
- [Building RPM Package](BUILD.md#rpm-package)
- [Building OpenSUSE Package](BUILD.md#opensuse)
- [Building from source](BUILD.md#Build-from-Source)
- [Debugging](BUILD.md#Debugging)


### Others

* Manjaro : https://github.com/hamonikr/nimf/wiki/Manjaro-build
* CentOS 8 : https://blog.naver.com/dfnk5516/222074913406
* Raspberry pi 4 arm64 : https://github.com/hamonikr/nimf/wiki/Install-nimf-on-raspberry-pi-4---arm64
* Armbian : https://github.com/hamonikr/nimf/wiki/Armbian-build
* Manjaro ARM : https://github.com/hamonikr/nimf/wiki/Manjaro-build
* Arch AUR : https://aur.archlinux.org/packages/nimf-git/
* Others : https://github.com/hamonikr/nimf/wiki/How-to-Build-and-Install-with-Others-Distro

<hr>

# 가볍고 빠른 입력기 프레임워크 nimf

[Go to English](#nimf)

이 프로젝트는 한글입력기 nimf 가 더이상 [지속되기 힘든 상황](https://launchpad.net/~hodong/+archive/ubuntu/nimf) 이 되었기 때문에

프로젝트의 지속적인 사용을 위해서는 관리가 필요하다고 생각되어 [nimf Project](https://gitlab.com/nimf-i18n/nimf) 를 포크한 프로젝트 입니다.

다년간 한글 사용자를 위한 환경 개선에 많은 기여를 하신 Hodong Kim 님께 감사를 드립니다. 

하모니카 개발팀은 개방형OS 배포에 필수적인 한글입력기에 대한 관리가 필요하다고 생각하고 있으며

앞으로 하모니카 팀에서 직접 nimf 프로젝트를 계속 관리하기로 결정하였습니다.

향후 하모니카 팀에서 이 프로젝트에 필요한 기능을 계속 추가하여 좋은 소프트웨어를 사용할 수 있도록 노력하겠습니다.

# nimf 설치

## Ubuntu(amd64, arm64), Debian(amd64, arm64), LinuxMint
```
# 우분투 21.10 이상을 기반으로 하는 배포판에서는 ibus-daemon이 자동으로 시작되어 입력기가 nimf 와 충돌됩니다.
# 부팅 시 ibus가 동작하지 않도록 아래의 방법으로 ibus를 제거하거나 ibus-daemon을 비활성화할 수 있습니다.
# 방법1 : sudo apt purge ibus
# 방법2 : sudo mv /usr/bin/ibus-daemon /usr/bin/ibus-daemon.bak

wget -qO- https://pkg.hamonikr.org/add-hamonikr.apt | sudo -E bash -

sudo apt install nimf nimf-libhangul

im-config -n nimf

# 만약 일본어, 중국어 등 다른 언어를 사용하고 싶은 경우에는 다음과 같이 추가 패키지를 설치해줍니다.
sudo apt install libnimf1 nimf nimf-anthy nimf-dev nimf-libhangul nimf-m17n nimf-rime
```
## Arch Linux, Manjaro
1) 패키지 다운로드 및 설치
```
# 최신 libhangul-git 패키지 설치
git clone https://aur.archlinux.org/libhangul-git.git

cd libhangul-git

makepkg -si 

# nimf 설치
git clone https://github.com/hamonikr/nimf.git

cd nimf

makepkg -si 
```

2) 입력기 설정
```
vi ~/.xprofile

export GTK_IM_MODULE=nimf
export QT4_IM_MODULE="nimf"
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"
```

## fedora, centos, redhat

 1) 패키지 다운로드 및 설치
```
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-1.fc40.x86_64.rpm

sudo yum install ~/nimf-*.rpm
``` 

## opensuse-leap

 1) 패키지 다운로드 및 설치
```
# 다른 언어도 포함하는 경우
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.x86_64.rpm

sudo rpm -ivh ./nimf-1.3.8-2.opensuse_leap.x86_64.rpm

# 한국어만 설치하는 경우
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

sudo rpm -ivh ./nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

``` 

# LICENSE
* GNU Lesser General Public License v3.0 ([한글 해석](https://olis.or.kr/license/Detailselect.do?lId=1073))
  
  Nimf is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Nimf is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program;  If not, see <http://www.gnu.org/licenses/>.

## ISSUE
사용 중 이슈는 깃허브 이슈를 이용하시거나 [하모니카 커뮤니티](https://hamonikr.org)를 방문해서 알려주시면 함께 고민하도록 하겠습니다.

For any issues you encounter while using this software, please use GitHub Issues or visit the [Hamonikr Community](https://hamonikr.org) to let us know so we can work together to address them.

## Contribution
깃허브 저장소를 포크하신 후 수정하실 내용을 수정하고 PR을 요청하시면 하모니카 팀에서 리뷰 후 반영됩니다.

To contribute, please fork the GitHub repository, make your changes, and submit a PR. The Hamonikr team will review and integrate your contributions.


