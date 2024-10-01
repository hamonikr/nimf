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

## Table of Contents
- [Nimf](#nimf)
  - [Features](#features)
  - [Supported Platforms](#supported-platforms)
  - [Installation](#installation)
    - [Ubuntu, Debian, LinuxMint](#ubuntu-debian-linuxmint)
    - [Arch Linux, Manjaro](#arch-linux-manjaro)
    - [Fedora, CentOS, RedHat](#fedora-centos-redhat)
    - [OpenSUSE Leap](#opensuse-leap)
  - [Configuration](#configuration)
  - [Build Instructions](#build-instructions)
  - [Additional Resources](#additional-resources)
  - [License](#license)
  - [Issue](#issue)
  - [Contribution](#contribution)  
- [Korean Section](#가볍고-빠른-입력기-프레임워크-nimf)


# nimf

Nimf is a lightweight, fast and extensible input method framework.

![nimf](docs/nimf.png)

### Features
- Input Method Server: `nimf`
- Language Engines: 
  - System Keyboard
  - Chinese (based on librime)
  - Japanese (based on anthy)
  - Korean (based on libhangul)
  - Various languages (based on m17n)
- Service Modules:
  - Indicator (based on appindicator)
  - Wayland, XIM (based on IMdkit)
  - Preedit window, Candidate window
- Client Modules:
  - GTK+2, GTK+3, Qt5, Qt6

### Supported Platforms
- **x86_64**
  - Ubuntu (>= 18.04)
  - Debian (>= 10)
  - Arch Linux (Rolling)
  - Fedora (>= 33)
  - openSUSE Leap (15.6)
- **arm64**
  - Ubuntu (>= 18.04)
  - Debian (>= 10)
  - Arch Linux (Rolling)

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

## opensuse-leap 15.6

 1) Install package gpg key from nimf repo.
 ```
 # Package GPG Key Download
 wget https://github.com/hamonikr/nimf/raw/master/RPM-GPG-KEY-nimf
 
 # Package GPG key installation
 sudo rpm --import RPM-GPG-KEY-nimf
 
 # Check the installed key
 rpm -qa gpg-pubkey --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" | grep e42665b8
 ```

 2) Download and Install
 
 The user can select and install the package you use, including a Korean package or including other languages package.
```
# When using other languages
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.x86_64.rpm

sudo zypper install ./nimf-1.3.8-2.opensuse_leap.x86_64.rpm

# If you want to install only Korean
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

sudo zypper install ./nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

``` 
 3) Reboot

 Use the nimf after the system restart.

## Build Instructions

For detailed build instructions, see the following sections in the [BUILD.md](BUILD.md) file:

- [Building Debian Package](BUILD.md#debian-package)
- [Building Arch Linux Package](BUILD.md#arch-linux-package)
- [Building RPM Package](BUILD.md#rpm-package)
- [Building OpenSUSE Package](BUILD.md#opensuse)
- [Building from source](BUILD.md#Build-from-Source)
- [Debugging](BUILD.md#Debugging)


### Additional Resources

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

## opensuse-leap 15.6

 1) nimf 저장소에서 GPG 키 설치

```
# GPG 키 다운로드
wget https://github.com/hamonikr/nimf/raw/master/RPM-GPG-KEY-nimf

# GPG 키 설치
sudo rpm --import RPM-GPG-KEY-nimf

# 설치된 키 확인
rpm -qa gpg-pubkey --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" | grep e42665b8
```

 2) 패키지 다운로드 및 설치

사용자는 한국어 패키지를 포함한 패키지 또는 다른 언어를 포함한 패키지를 선택하여 설치할 수 있습니다.

```
# 다른 언어를 사용할 경우
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.x86_64.rpm

sudo zypper install ./nimf-1.3.8-2.opensuse_leap.x86_64.rpm

# 한국어만 설치하고 싶은 경우
wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm

sudo zypper install ./nimf-1.3.8-2.opensuse_leap.kr.x86_64.rpm
```

 3) 재부팅

시스템을 재시작한 후 nimf를 사용하십시오.


# 한글 자판 배열 추가

libhangul에서 사용하는 한글 자판 배열은 두 경로에 XML 파일로 정의됩니다.

* `/usr/share/libhangul/keyboards` 경로에 설치된 파일은 모든 사용자에게서 인식됩니다.
* `$HOME/.local/share/libhangul/keyboards` 또는 `$XDG_DATA_HOME/libhangul/keyboards` 경로에 설치된 파일은 개별 사용자에게 인식됩니다.

자판 배열 파일 구조는 기본으로 설치되는 [두벌식 배열 파일](docs/hangul-keyboard-2.xml)과 [자모 기본조합 파일](docs/hangul-combination-default.xml)를 참고하세요. 일반적인 구조는 다음과 같습니다.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<hangul-keyboard id="2" type="jamo">

    <name>Dubeolsik</name>
    <name xml:lang="ko">두벌식</name>

    <map id="0">
        <item key="0x41" value="0x1106"/>  <!-- A → ᄆᅠ -->
        <item key="0x42" value="0x1172"/>  <!-- B → ᅟᅲ -->
        ...
    </map>

    <combination id="0">
        <item first="0x1100" second="0x1100" result="0x1101"/>  <!-- ᄀ   + ᄀ   → ᄁ  -->
        <item first="0x1169" second="0x1161" result="0x116a"/>  <!-- ᅩ   + ᅡ   → ᅪ  -->
        ...
    </combination>
    <!-- 또는 -->
    <include file="hangul-combination-default.xml"/>

</hangul-keyboard>
```

* hangul-keyboard: 자판 배열을 정의하는 루트 요소.
    * id: 입력기에서 사용하는 ID 값.
    * type: 자판의 유형에 따라 jamo(두벌식), jamo-yet(두벌식 옛한글), jaso(세벌식), jaso-yet(세벌식 옛한글), ro(로마자)로 설정합니다.
* name: 설정 창에 나타나는 자판 배열의 이름. xml:lang 값에 따라 지역화된 이름을 설정할 수 있습니다.
* map: 각 키를 눌렀을 때 입력될 문자를 설정합니다. id 값은 0으로 고정합니다.
    * item: QWERTY 자판 기준 아스키코드 key에 해당하는 키를 누르면, 유니코드 value에 해당하는 한글 초/중/종성을 입력합니다. key와 value 값은 16진수로 지정합니다.
* combination: 키를 연달아 누를 때 입력될 조합자를 item으로 나열합니다. id 값은 0으로 고정합니다.
    * item: first 문자가 입력된 상태에서 second 키를 누르면 result 문자로 바뀝니다. first, second, result는 한글 초/중/종성에 대응되는 16진수 유니코드로 지정합니다.
* include: file 경로의 XML 파일을 읽어 그 자리에 인라인합니다. file 값은 상대경로 또는 절대경로로 설정할 수 있습니다.

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


