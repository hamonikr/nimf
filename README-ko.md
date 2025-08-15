# 가볍고 빠른 입력기 프레임워크 nimf

[![Build Status](https://github.com/hamonikr/nimf/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/hamonikr/nimf/actions/workflows/build.yml)
[![Donate Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://ko.liberapay.com/hamonikr/)

![version](https://img.shields.io/badge/version-1.3.10-blue)
![toolkits](https://img.shields.io/badge/GTK-2%2F3%2F4-green)
![qt](https://img.shields.io/badge/Qt-5%2F6-green)
![arch](https://img.shields.io/badge/arch-x86__64%20%7C%20arm64-darkblue)
![ubuntu](https://img.shields.io/badge/Ubuntu-24.04%2B-orange)
![debian](https://img.shields.io/badge/Debian-Bookworm%2FTrixie-brown)
![fedora](https://img.shields.io/badge/Fedora-33%2B-blue)
![opensuse](https://img.shields.io/badge/openSUSE-Leap%2015.6-green)
![archlinux](https://img.shields.io/badge/Arch-Rolling-blue)

[🇺🇸 English](README.md) | **🇰🇷 한국어**

---

이 프로젝트는 한글입력기 nimf가 더이상 [지속되기 힘든 상황](https://launchpad.net/~hodong/+archive/ubuntu/nimf)이 되었기 때문에 프로젝트의 지속적인 사용을 위해 [nimf Project](https://gitlab.com/nimf-i18n/nimf)를 포크한 프로젝트입니다.

하모니카 개발팀은 개방형 OS 배포에 필수적인 한글 입력기에 대한 관리가 필요하다고 생각하며 앞으로 하모니카 팀에서 직접 nimf 프로젝트를 계속 관리합니다.

![nimf](docs/nimf.png)

## 📋 목차

- [프로젝트 소개](#-프로젝트-소개)
- [1.3.10의 주요 변경사항](#-1310의-주요-변경사항)
- [주요 기능](#-주요-기능)
- [지원 플랫폼](#-지원-플랫폼)
- [설치 방법](#-설치-방법)
- [소스에서 빌드](#-소스에서-빌드)
- [한글 자판 배열 추가](#-한글-자판-배열-추가)
- [라이선스](#-라이선스)
- [이슈 및 기여](#-이슈-및-기여)

## 🎯 프로젝트 소개

Nimf는 가볍고 빠르며 확장 가능한 입력기 프레임워크입니다.

- **가벼움**: 시스템 리소스를 최소한으로 사용
- **빠름**: 반응속도가 빠른 입력 처리
- **확장성**: 다양한 언어와 플랫폼 지원
- **호환성**: GTK2/3/4, Qt5/6 완전 지원

## 🆕 1.3.10의 주요 변경사항

- **GTK4 완전 지원** (GTK2/GTK3/GTK4 동시 환경 지원)
- **데비안 패키지 분리**:
  - `nimf`: 코어 + 한글(libhangul), 일본어(anthy), 중국어(rime)
  - `nimf-i18n`: 다국어 엔진(m17n) 추가 패키지
- **빌드 옵션 업데이트**:
  - `--enable-gtk4`: GTK4 IM 모듈 빌드
  - `--with-gtk=3|4`: `nimf-settings`의 GTK 버전 선택
  - 엔진 토글: `--disable-nimf-{libhangul,anthy,m17n,rime}`
  - `--disable-x11`: Wayland 전용 빌드
  - 통합 옵션: `--with-im-config-data`, `--with-imsettings-data`

## ⚡ 주요 기능

### 입력기 서버
- `nimf`: 메인 입력기 데몬

### 언어 엔진
- **시스템 키보드**: 기본 키보드 입력
- **한글 입력**: libhangul 기반 (두벌식, 세벌식 등)
- **일본어 입력**: anthy 기반
- **중국어 입력**: librime 기반
- **다국어 입력**: m17n 기반 (`nimf-i18n` 설치 시)

### 서비스 모듈
- **인디케이터**: ayatana-appindicator 기반 시스템 트레이
- **프로토콜 지원**: Wayland, XIM (IMdkit 기반)
- **UI 구성요소**: 미리보기창, 후보창

### 클라이언트 모듈
- **GTK 지원**: GTK+2, GTK+3, GTK4
- **Qt 지원**: Qt5, Qt6

## 🖥️ 지원 플랫폼

### x86_64 아키텍처
- Ubuntu (>= 24.04)
- Debian (>= 12 Bookworm)
- Arch Linux (Rolling)
- Fedora (>= 33)
- openSUSE Leap (15.6)

### arm64 아키텍처
- Ubuntu (>= 24.04)
- Debian (>= 12 Bookworm)
- Arch Linux (Rolling)

## 🚀 설치 방법

### 자동 설치 (권장)

한 줄 명령으로 최신 버전을 설치합니다:

```bash
# curl 사용
curl -fsSL https://raw.githubusercontent.com/hamonikr/nimf/master/install | sudo bash

# 또는 wget 사용
wget -qO- https://raw.githubusercontent.com/hamonikr/nimf/master/install | sudo -E bash -
```

이 스크립트는 자동으로:
- 배포판을 감지하고 적절한 패키지를 설치
- **ibus 충돌을 자동 처리** (ibus-daemon이 있으면 비활성화)
- 모든 데스크톱 환경에 환경 변수 설정
- Nimf를 기본 입력기로 설정

### 수동 설치

#### Ubuntu 24.04+, Debian 12+ (amd64, arm64), LinuxMint

Ubuntu 21.10 이상에서는 `ibus-daemon`이 자동 시작되어 nimf와 충돌할 수 있습니다.

**방법 1: ibus 제거**
```bash
sudo apt purge ibus
```

**방법 2: ibus-daemon 비활성화**
```bash
sudo mv /usr/bin/ibus-daemon /usr/bin/ibus-daemon.bak
```

**Nimf 설치**
```bash
curl -fsSL https://repo.hamonikr.org/install | sudo bash

sudo apt install nimf

im-config -n nimf
```

**다국어 엔진 추가(m17n)**
```bash
sudo apt install nimf-i18n
```

#### Arch Linux, Manjaro

**방법 A: makepkg로 빌드 설치**
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

**방법 B: 릴리스 패키지 설치**
```bash
# 릴리스 페이지에서 최신 패키지 다운로드 후 설치
sudo pacman -U ./nimf-*.pkg.tar.zst
```

#### Fedora, CentOS, RedHat

**릴리스(RPM)에서 설치**
```bash
# 릴리스 페이지에서 최신 .rpm 다운로드 후 설치
sudo dnf install ./nimf-*.rpm
```

#### openSUSE Leap 15.6

**1) nimf 저장소 GPG 키 설치**
```bash
wget https://github.com/hamonikr/nimf/raw/master/RPM-GPG-KEY-nimf
sudo rpm --import RPM-GPG-KEY-nimf
rpm -qa gpg-pubkey --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" | grep e42665b8
```

**2) 패키지 설치**
```bash
# 릴리스 페이지에서 최신 .rpm 다운로드 후 설치
sudo zypper install ./nimf-*.rpm
```

**3) 재부팅**

### 설정

필요 시 셸 프로파일에 입력기 모듈 설정을 추가합니다:

```bash
export GTK_IM_MODULE=nimf
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"

# GTK4 앱을 사용하는 경우(선택)
export GTK4_IM_MODULE=nimf
```

## 🔧 소스에서 빌드

### Docker 기반 패키지 빌드 (권장)

모든 지원 플랫폼에 대한 Docker 기반 빌드를 제공합니다:

```bash
git clone --recurse-submodules https://github.com/hamonikr/nimf
cd nimf

# 특정 플랫폼용 빌드
./scripts/build-docker.sh debian.bookworm
./scripts/build-docker.sh debian.trixie
./scripts/build-docker.sh ubuntu.2404
./scripts/build-docker.sh fedora.latest
./scripts/build-docker.sh opensuse
./scripts/build-docker.sh arch

# 생성된 패키지는 dist/ 폴더에 위치
find dist/ -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*"
```

자세한 Docker 빌드 방법은 [DOCKER-BUILD.md](DOCKER-BUILD.md)를 참고하세요.

### 소스 빌드

배포판별 패키지 생성 방법은 [BUILD.md](BUILD.md)를 참고하세요.

**소스에서 빌드(요약):**
```bash
git clone --recurse-submodules https://github.com/hamonikr/nimf
cd nimf
./autogen.sh
./configure --prefix=/usr/local
make -j "$(nproc)"
sudo make install
```

**주요 설정 옵션:**
- `--enable-gtk4`: GTK4 IM 모듈 빌드(기본 자동 감지)
- `--with-gtk=3|4`: `nimf-settings`의 GTK 버전 선택(기본 3)
- `--disable-nimf-libhangul`, `--disable-nimf-anthy`, `--disable-nimf-m17n`, `--disable-nimf-rime`
- `--disable-x11`: X11 제외(Wayland 전용)
- `--with-im-config-data`, `--with-imsettings-data`: 통합 데이터 설치

### 릴리즈 관리

버전 관리를 위한 릴리즈 스크립트 사용:

```bash
# 패치 버전 증가 (자동)
./scripts/release.sh -y

# 마이너 버전 증가  
./scripts/release.sh minor

# 메이저 버전 증가
./scripts/release.sh major
```

스크립트가 자동으로:
- `configure.ac`, `PKGBUILD`, `nimf.spec`, `debian/changelog`의 버전 업데이트
- 커밋 및 태그 생성
- GitHub Actions 릴리즈 워크플로우 트리거를 위한 푸시

## ⌨️ 한글 자판 배열 추가

libhangul에서 사용하는 한글 자판 배열은 다음 경로에 XML 파일로 정의됩니다:

- **시스템 전체**: `/usr/share/libhangul/keyboards`
- **사용자별**: `$HOME/.local/share/libhangul/keyboards` 또는 `$XDG_DATA_HOME/libhangul/keyboards`

기본 제공 예제를 참고하세요:
- `docs/hangul-keyboard-2.xml`
- `docs/hangul-combination-default.xml`

### 일반적인 구조

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

### 요소 설명

- **`hangul-keyboard`**: 루트 요소
  - `id`: 입력기에서 사용하는 식별자
  - `type`: `jamo`, `jamo-yet`, `jaso`, `jaso-yet`, `ro`
- **`name`**: 표시 이름, `xml:lang`로 지역화 가능
- **`map`**: 키-유니코드 매핑(항상 id=0)
- **`combination`**: 연속 조합 규칙(항상 id=0)
- **`include`**: 다른 XML을 경로로 인라인 포함

## 📜 라이선스

**GNU Lesser General Public License v3.0** ([한글 해석](https://olis.or.kr/license/Detailselect.do?lId=1073))

Nimf is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nimf is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program; If not, see <http://www.gnu.org/licenses/>.

## 🤝 이슈 및 기여

### 이슈 신고

사용 중 이슈는 다음 방법으로 알려주세요:
- [GitHub Issues](https://github.com/hamonikr/nimf/issues) 사용
- [하모니카 커뮤니티](https://hamonikr.org) 방문

### 기여하기

깃허브 저장소를 포크하신 후 수정하실 내용을 수정하고 PR을 요청하시면 하모니카 팀에서 리뷰 후 반영됩니다.

**기여 과정:**
1. 저장소 포크
2. 기능 브랜치 생성
3. 변경사항 커밋
4. Pull Request 생성
5. 코드 리뷰 및 피드백
6. 최종 병합

### 추가 자료

- Docker Build Guide: [DOCKER-BUILD.md](DOCKER-BUILD.md)
- 전체 빌드 가이드: [BUILD.md](BUILD.md)
- Manjaro: <https://github.com/hamonikr/nimf/wiki/Manjaro-build>
- CentOS 8: <https://blog.naver.com/dfnk5516/222074913406>
- Raspberry Pi 4 arm64: <https://github.com/hamonikr/nimf/wiki/Install-nimf-on-raspberry-pi-4---arm64>
- Armbian: <https://github.com/hamonikr/nimf/wiki/Armbian-build>
- Manjaro ARM: <https://github.com/hamonikr/nimf/wiki/Manjaro-build>
- Arch AUR: <https://aur.archlinux.org/packages/nimf-git/>
- 기타 배포판: <https://github.com/hamonikr/nimf/wiki/How-to-Build-and-Install-with-Others-Distro>

---

**하모니카팀에서 관리하는 오픈소스 프로젝트입니다** 💙