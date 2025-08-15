#!/bin/bash
set -e
cd /home/builduser/src

# 패키지 빌드 의존성 설치
pacman -Sy --noconfirm base-devel

# 버전 정보 업데이트
VERSION=1.3.8
sed -i "s/pkgver=.*/pkgver=${VERSION}/" PKGBUILD
sed -i "s/pkgrel=.*/pkgrel=1/" PKGBUILD

# PKGBUILD 내용 확인
echo "=== PKGBUILD 내용 확인 ==="
head -20 PKGBUILD

# 일반 사용자로 패키지 빌드
chown -R builduser:builduser /home/builduser/src
su builduser -c 'makepkg --noconfirm'

# 패키지 파일 복사
mkdir -p /packages/arch
find . -name "*.pkg.tar.*" -exec cp {} /packages/arch/ \;

echo "=== 생성된 Arch 패키지 ==="
ls -la /packages/arch/
