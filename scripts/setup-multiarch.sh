#!/bin/bash

# ARM64 빌드를 위한 로컬 환경 설정 스크립트

echo "🔧 ARM64 빌드를 위한 로컬 환경 설정 중..."

# QEMU 설치 확인
if ! command -v qemu-user-static &> /dev/null; then
    echo "📦 QEMU 설치 중..."
    sudo apt-get update
    sudo apt-get install -y qemu-user-static binfmt-support
fi

# Docker Buildx 설정
echo "🐳 Docker Buildx 설정 중..."
docker buildx create --name multiarch --driver docker-container --bootstrap --use 2>/dev/null || echo "Builder already exists"

# QEMU 등록
echo "⚙️ QEMU 등록 중..."
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

# 지원 플랫폼 확인
echo "✅ 지원 플랫폼 확인:"
docker buildx ls

echo "🎉 ARM64 빌드 환경 설정 완료!"
echo ""
echo "이제 다음 명령으로 ARM64 빌드를 테스트할 수 있습니다:"
echo "  ./scripts/build-docker-multiarch.sh ubuntu.2404.arm64"