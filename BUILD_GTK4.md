# Nimf GTK4 지원 빌드 가이드

## 개요
Nimf 1.3.9 버전부터 GTK4를 완전히 지원합니다. 이 문서는 GTK3와 GTK4를 모두 지원하는 nimf 패키지를 빌드하는 방법을 설명합니다.

## 주요 변경사항

### 1. GTK4 지원 추가
- GTK4 전용 IM 모듈 (`im-nimf-gtk4.so`) 구현
- GTK3와 GTK4 동시 지원
- Wayland 환경 개선

### 2. 빌드 시스템 업데이트
- `configure.ac`에 GTK4 옵션 추가
- 데비안 패키징에 GTK4 지원 포함

## 빌드 방법

### 소스에서 직접 빌드
```bash
# 의존성 설치
sudo apt install libgtk-4-dev libgtk-4-bin

# configure 실행 (GTK4 지원 활성화)
./autogen.sh
./configure --enable-gtk4

# 빌드
make
sudo make install
```

### 데비안 패키지 빌드
```bash
# 빌드 의존성 설치
sudo apt build-dep nimf
sudo apt install libgtk-4-dev libgtk-4-bin

# 패키지 빌드 (GTK4 자동 포함)
dpkg-buildpackage -us -uc -b

# 패키지 설치
sudo dpkg -i ../nimf_1.3.9_amd64.deb
```

## 환경 설정

### 기본 설정 (GTK2/GTK3/Qt)
```bash
export GTK_IM_MODULE=nimf
export QT_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"
```

### GTK4 애플리케이션용 추가 설정
```bash
export GTK4_IM_MODULE=nimf
```

## 설치된 파일 확인
```bash
# GTK2 모듈
/usr/lib/x86_64-linux-gnu/gtk-2.0/2.10.0/immodules/im-nimf-gtk2.so

# GTK3 모듈
/usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-nimf-gtk3.so

# GTK4 모듈 (새로 추가)
/usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules/im-nimf-gtk4.so
```

## Wayland 지원
- X11과 Wayland 환경 자동 감지
- Wayland 환경에서는 GDK_BACKEND를 강제하지 않음
- 더 나은 호환성 제공

## 테스트
```bash
# GTK4 애플리케이션에서 한글 입력 테스트
gtk4-demo

# 환경변수 확인
echo $GTK_IM_MODULE
echo $GTK4_IM_MODULE
```

## 문제 해결

### GTK4 모듈이 로드되지 않는 경우
```bash
# IM 모듈 캐시 업데이트
sudo gtk4-query-immodules --update-cache
```

### Wayland에서 입력이 안 되는 경우
```bash
# X11 백엔드 강제 (임시 해결책)
export GDK_BACKEND=x11
```

## 호환성
- Ubuntu 22.04 이상
- Debian 12 이상
- HamoniKR 8.0 이상
- GTK 4.6 이상