# Docker 기반 다중 플랫폼 패키지 빌드 가이드

이 문서는 Docker를 사용하여 Nimf를 다양한 Linux 배포판용 패키지로 빌드하는 방법을 설명합니다.

## 지원 플랫폼

### DEB 패키지 (Debian/Ubuntu)
- Ubuntu 24.04 (Noble)
- Debian Bookworm
- Debian Trixie

### RPM 패키지 (Red Hat 계열)
- Fedora Latest
- OpenSUSE Leap

### Arch 패키지
- Arch Linux

## 사전 요구사항

- Docker 설치 및 실행
- Git (소스 코드 관리용)
- 충분한 디스크 공간 (빌드 과정에서 여러 Docker 이미지 생성)

## 빌드 방법

### 1. 자동 빌드 스크립트 사용

프로젝트 루트에서 다음 명령을 실행하세요:

```bash
# 특정 플랫폼용 패키지 빌드
./scripts/build-docker.sh debian.bookworm
./scripts/build-docker.sh debian.trixie
./scripts/build-docker.sh ubuntu.2404
./scripts/build-docker.sh fedora.latest
./scripts/build-docker.sh opensuse
./scripts/build-docker.sh arch

# 생성된 패키지 확인
find dist/ -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*"
```

### 2. 수동 빌드

특정 배포판용 패키지를 수동으로 빌드하려면:

#### Ubuntu 24.04 DEB 패키지
```bash
# Docker 이미지 빌드
docker build -f .docker/ubuntu.2404.Dockerfile -t nimf-builder:ubuntu2404 .

# 패키지 빌드
docker run --rm -v $PWD/dist:/packages --entrypoint="/bin/bash" nimf-builder:ubuntu2404 -c "
    cd /src
    export DEBIAN_FRONTEND=noninteractive
    debchange --newversion '1.3.8-1' --distribution unstable 'Release version 1.3.8'
    debuild -us -uc -b
    mkdir -p /packages/ubuntu.2404
    cp ../*.deb /packages/ubuntu.2404/
"
```

#### Fedora RPM 패키지
```bash
# Docker 이미지 빌드
docker build -f .docker/fedora.latest.Dockerfile -t nimf-builder:fedora .

# 패키지 빌드
docker run --rm -v $PWD/dist:/packages --entrypoint="/bin/bash" nimf-builder:fedora -c "
    cd /src
    dnf install -y rpm-build rpmdevtools
    mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    VERSION=1.3.8
    tar czf ~/rpmbuild/SOURCES/nimf-\${VERSION}.tar.gz --transform 's,^,nimf-\${VERSION}/,' --exclude=dist --exclude=.git *
    cp nimf.spec ~/rpmbuild/SPECS/
    rpmbuild -ba ~/rpmbuild/SPECS/nimf.spec
    mkdir -p /packages/fedora.latest
    find ~/rpmbuild/RPMS -name '*.rpm' -exec cp {} /packages/fedora.latest/ \;
"
```

#### Arch Linux 패키지
```bash
# Docker 이미지 빌드
docker build -f .docker/arch.Dockerfile -t nimf-builder:arch .

# 패키지 빌드
docker run --rm -v $PWD/dist:/packages --entrypoint="/bin/bash" nimf-builder:arch -c "
    cd /home/builduser/src
    pacman -Sy --noconfirm base-devel
    chown -R builduser:builduser /home/builduser/src
    su builduser -c 'makepkg --noconfirm'
    mkdir -p /packages/arch
    find . -name '*.pkg.tar.*' -exec cp {} /packages/arch/ \;
"
```

#### 🛠️ 로컬 ARM64 빌드 테스트 방법

  1. 멀티 아키텍처 환경 설정

  # 한 번만 실행하면 됩니다
  ./scripts/setup-multiarch.sh

  2. ARM64 빌드 테스트

  # 기존 스크립트 대신 멀티아키텍처 스크립트 사용
  ./scripts/build-docker-multiarch.sh ubuntu.2404.arm64
  ./scripts/build-docker-multiarch.sh debian.bookworm.arm64
  ./scripts/build-docker-multiarch.sh arch.arm64


## 빌드 결과

빌드가 완료되면 `dist/` 디렉토리에 다음과 같은 구조로 패키지들이 생성됩니다:

```
dist/
├── debian.bookworm/     # Debian Bookworm DEB 패키지들
│   ├── nimf_1.3.8-1_amd64.deb
│   └── nimf-i18n_1.3.8-1_amd64.deb
├── debian.trixie/       # Debian Trixie DEB 패키지들
│   ├── nimf_1.3.8-1_amd64.deb
│   └── nimf-i18n_1.3.8-1_amd64.deb
├── ubuntu.2404/         # Ubuntu 24.04 DEB 패키지들
│   ├── nimf_1.3.8-1_amd64.deb
│   └── nimf-i18n_1.3.8-1_amd64.deb
├── fedora.latest/       # Fedora RPM 패키지들
│   ├── nimf-1.3.8-1.fc40.x86_64.rpm
│   └── nimf-1.3.8-1.fc40.src.rpm
├── opensuse/            # OpenSUSE RPM 패키지들
│   └── nimf-1.3.8-1.x86_64.rpm
└── arch/                # Arch 패키지들
    └── nimf-1.3.8-1-any.pkg.tar.zst
```

## 패키지 설치

### Ubuntu/Debian
```bash
sudo dpkg -i dist/ubuntu.2404/nimf_*.deb
# 또는
sudo dpkg -i dist/debian.bookworm/nimf_*.deb
sudo apt-get install -f  # 의존성 해결
```

### Fedora
```bash
sudo dnf install dist/fedora.latest/nimf-*.rpm
```

### OpenSUSE
```bash
sudo zypper install dist/opensuse/nimf-*.rpm
```

### Arch Linux
```bash
sudo pacman -U dist/arch/nimf-*.pkg.tar.zst
```

## 문제 해결

### Docker 권한 문제
사용자가 docker 그룹에 속하지 않은 경우:
```bash
sudo usermod -aG docker $USER
# 로그아웃 후 다시 로그인
```

### 디스크 공간 부족
빌드 후 불필요한 Docker 이미지 정리:
```bash
docker system prune -a
```

### 빌드 실패
특정 배포판에서 빌드가 실패하는 경우, 해당 Docker 파일의 의존성을 확인하세요:
```bash
# 예: Ubuntu 24.04 Docker 파일 확인
cat .docker/ubuntu.2404.Dockerfile
```

### 패키지 경로 오류
생성된 패키지가 예상 위치에 없는 경우:
```bash
# dist 디렉토리 전체 검색
find dist/ -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*"

# 특정 플랫폼 디렉토리 확인
ls -la dist/ubuntu.2404/
ls -la dist/fedora.latest/
ls -la dist/arch/
```

## 기여하기

새로운 배포판 지원을 추가하려면:

1. `.docker/` 디렉토리에 새 Dockerfile 생성
2. `scripts/build-docker.sh`에서 새 배포판 지원 확인
3. `.github/workflows/release.yml`에 빌드 작업 추가
4. 테스트 후 Pull Request 제출

## 라이선스

이 빌드 시스템은 Nimf 프로젝트와 동일한 라이선스를 따릅니다.
