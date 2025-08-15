#!/bin/bash

# 멀티 아키텍처 빌드를 위한 개선된 Docker 빌드 스크립트

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 스크립트의 실제 위치 확인
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# 프로젝트 루트 디렉토리로 이동
cd "$PROJECT_ROOT"

echo -e "${YELLOW}NIMF 멀티 아키텍처 Docker 빌드 스크립트${NC}"
echo -e "${YELLOW}프로젝트 디렉토리: ${NC}$PROJECT_ROOT"

# 함수: 사용법 출력
show_usage() {
    echo -e "\n${YELLOW}사용법:${NC}"
    echo -e "  $0 [dockerfile_name]"
    echo -e "\n${YELLOW}예제:${NC}"
    echo -e "  $0 ubuntu.2404.arm64     # Ubuntu 24.04 ARM64 빌드"
    echo -e "  $0 debian.bookworm.arm64 # Debian Bookworm ARM64 빌드"
    echo -e "  $0 arch.arm64            # Arch Linux ARM64 빌드"
    echo -e "\n${YELLOW}주의사항:${NC}"
    echo -e "  ARM64 빌드 전에 먼저 ./scripts/setup-multiarch.sh를 실행하세요."
}

# 파라미터 확인
if [ $# -ne 1 ]; then
    echo -e "${RED}Error: 정확히 하나의 Dockerfile 이름이 필요합니다.${NC}"
    show_usage
    exit 1
fi

DOCKERFILE_NAME=$1
DOCKERFILE_PATH=".docker/${DOCKERFILE_NAME}.Dockerfile"

# Dockerfile 존재 확인
if [ ! -f "$DOCKERFILE_PATH" ]; then
    echo -e "${RED}Error: $DOCKERFILE_PATH 파일을 찾을 수 없습니다.${NC}"
    exit 1
fi

# Docker Buildx 확인
if ! docker buildx ls | grep -q multiarch; then
    echo -e "${RED}Error: multiarch builder가 설정되지 않았습니다.${NC}"
    echo -e "${YELLOW}먼저 ./scripts/setup-multiarch.sh를 실행하세요.${NC}"
    exit 1
fi

# 아키텍처 감지
if [[ "$DOCKERFILE_NAME" == *"arm64"* ]]; then
    PLATFORM="linux/arm64"
    ARCH="arm64"
else
    PLATFORM="linux/amd64"
    ARCH="amd64"
fi

IMAGE_NAME="nimf-test-${DOCKERFILE_NAME}"

echo -e "\n${BLUE}=== $DOCKERFILE_NAME 환경 테스트 시작 ===${NC}"
echo -e "${YELLOW}플랫폼: $PLATFORM${NC}"

# Docker 이미지 빌드
echo -e "${YELLOW}Docker 이미지 빌드 중...${NC}"
if ! docker buildx build \
    --platform "$PLATFORM" \
    --tag "$IMAGE_NAME" \
    --file "$DOCKERFILE_PATH" \
    --load \
    .; then
    echo -e "${RED}Error: Docker 빌드 실패 - $DOCKERFILE_NAME${NC}"
    exit 1
fi

echo -e "${GREEN}빌드 완료 - $DOCKERFILE_NAME${NC}"

# 패키지 생성 (Debian/Ubuntu의 경우)
if [[ "$DOCKERFILE_NAME" == *"ubuntu"* ]] || [[ "$DOCKERFILE_NAME" == *"debian"* ]]; then
    echo -e "${YELLOW}DEB 패키지 생성 중...${NC}"
    docker run --rm \
        --platform "$PLATFORM" \
        -v "$PROJECT_ROOT:/packages" \
        --entrypoint="/bin/bash" \
        "$IMAGE_NAME" -c "
        cd /src && 
        debuild -us -uc &&
        mkdir -p /packages/dist/$DOCKERFILE_NAME &&
        find .. -name '*.deb' -exec cp {} /packages/dist/$DOCKERFILE_NAME/ \; && 
        echo 'DEB 패키지 생성 완료:' && 
        ls -la /packages/dist/$DOCKERFILE_NAME/*.deb 2>/dev/null || echo 'No .deb files created'
    "
fi

# Arch 패키지 생성
if [[ "$DOCKERFILE_NAME" == *"arch"* ]]; then
    echo -e "${YELLOW}Arch 패키지 생성 중...${NC}"
    docker run --rm \
        --platform "$PLATFORM" \
        -v "$PROJECT_ROOT:/packages" \
        --entrypoint="/bin/bash" \
        "$IMAGE_NAME" -c "
        cd /home/builduser/src && 
        su - builduser -c 'cd /home/builduser/src && makepkg -sf --noconfirm' &&
        mkdir -p /packages/dist/$DOCKERFILE_NAME &&
        cp /home/builduser/src/*.pkg.tar.* /packages/dist/$DOCKERFILE_NAME/ 2>/dev/null &&
        echo 'Arch 패키지 생성 완료:' &&
        ls -la /packages/dist/$DOCKERFILE_NAME/*.pkg.tar.* 2>/dev/null || echo 'No Arch packages created'
    "
fi

# 생성된 패키지 파일 목록 표시
echo -e "\n${BLUE}=== 생성된 패키지 파일 ===${NC}"
find "$PROJECT_ROOT/dist/$DOCKERFILE_NAME" -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*" 2>/dev/null | sort

echo -e "${GREEN}테스트 완료 - $DOCKERFILE_NAME${NC}"