#!/bin/bash

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

echo -e "${YELLOW}NIMF Docker 빌드 테스트 스크립트${NC}"
echo -e "${YELLOW}프로젝트 디렉토리: ${NC}$PROJECT_ROOT"

# 사용 가능한 Dockerfile 목록
AVAILABLE_DOCKERFILES=(
    "ubuntu.2204.Dockerfile"
    "ubuntu.2404.Dockerfile" 
    "ubuntu.2404.arm64.Dockerfile"
    "debian.bookworm.Dockerfile"
    "debian.bookworm.arm64.Dockerfile"
    "debian.trixie.Dockerfile"
    "debian.trixie.arm64.Dockerfile"
    "debian.sid.Dockerfile"
    "fedora.40.Dockerfile"
    "fedora.latest.Dockerfile"
    "fedora.latest.arm64.Dockerfile"
    "opensuse.Dockerfile"
    "opensuse.arm64.Dockerfile"
    "arch.Dockerfile"
)

# 함수: 사용법 출력
show_usage() {
    echo -e "\n${YELLOW}사용법:${NC}"
    echo -e "  $0 [dockerfile] [options]"
    echo -e "\n${YELLOW}사용 가능한 Dockerfile:${NC}"
    for dockerfile in "${AVAILABLE_DOCKERFILES[@]}"; do
        echo -e "  - ${dockerfile%.Dockerfile}"
    done
    echo -e "\n${YELLOW}옵션:${NC}"
    echo -e "  --build-only    : 빌드만 수행 (패키지 생성 안함)"
    echo -e "  --package-only  : 패키지만 생성 (빌드 스킵)"
    echo -e "  --interactive   : 빌드 후 대화형 셸 실행"
    echo -e "  --clean        : 기존 이미지 삭제 후 새로 빌드"
    echo -e "  --all          : 모든 환경에서 테스트"
    echo -e "\n${YELLOW}예제:${NC}"
    echo -e "  $0 debian.trixie           # Debian Trixie에서 빌드 및 패키지 생성"
    echo -e "  $0 ubuntu.2404 --interactive  # Ubuntu 24.04에서 빌드 후 셸 실행"
    echo -e "  $0 --all --build-only      # 모든 환경에서 빌드만 테스트"
}

# 함수: Docker 빌드 및 테스트
test_dockerfile() {
    local dockerfile_name=$1
    local build_only=${2:-false}
    local package_only=${3:-false}
    local interactive=${4:-false}
    local clean=${5:-false}
    
    local dockerfile_path=".docker/${dockerfile_name}.Dockerfile"
    local image_name="nimf-test-${dockerfile_name}"
    
    if [ ! -f "$dockerfile_path" ]; then
        echo -e "${RED}Error: $dockerfile_path 파일을 찾을 수 없습니다.${NC}"
        return 1
    fi
    
    echo -e "\n${BLUE}=== $dockerfile_name 환경 테스트 시작 ===${NC}"
    
    # 기존 이미지 삭제
    if [ "$clean" = true ]; then
        echo -e "${YELLOW}기존 이미지 삭제 중...${NC}"
        docker rmi "$image_name" 2>/dev/null || true
    fi
    
    # Docker 이미지 빌드
    echo -e "${YELLOW}Docker 이미지 빌드 중...${NC}"
    if ! docker build -t "$image_name" -f "$dockerfile_path" .; then
        echo -e "${RED}Error: Docker 빌드 실패 - $dockerfile_name${NC}"
        return 1
    fi
    
    if [ "$build_only" = true ]; then
        echo -e "${GREEN}빌드 완료 - $dockerfile_name${NC}"
        return 0
    fi
    
    if [ "$package_only" = true ]; then
        echo -e "${YELLOW}빌드를 건너뛰고 패키지 생성만 수행합니다...${NC}"
    fi
    
    # 패키지 생성 (Debian/Ubuntu의 경우)
    if [[ "$dockerfile_name" == *"ubuntu"* ]] || [[ "$dockerfile_name" == *"debian"* ]]; then
        echo -e "${YELLOW}DEB 패키지 생성 중...${NC}"
        if ! docker run --rm -v "$PROJECT_ROOT:/packages" --entrypoint="/bin/bash" "$image_name" -c "
            cd /src && 
            debuild -us -uc &&
            mkdir -p /packages/dist/$dockerfile_name &&
            find .. -name '*.deb' -exec cp {} /packages/dist/$dockerfile_name/ \; && 
            echo -e '${GREEN}DEB 패키지 생성 완료:${NC}' && 
            ls -la /packages/dist/$dockerfile_name/*.deb 2>/dev/null || echo 'No .deb files created'
        "; then
            echo -e "${RED}Warning: DEB 패키지 생성 중 오류 발생 - $dockerfile_name${NC}"
        else
            echo -e "${GREEN}DEB 패키지가 $PROJECT_ROOT/dist/$dockerfile_name 디렉토리에 저장되었습니다.${NC}"
        fi
    fi
    
    # RPM 패키지 생성 (Fedora/OpenSUSE의 경우)
    if [[ "$dockerfile_name" == *"fedora"* ]] || [[ "$dockerfile_name" == *"opensuse"* ]]; then
        echo -e "${YELLOW}RPM 패키지 생성 중...${NC}"
        if ! docker run --rm -v "$PROJECT_ROOT:/packages" --entrypoint="/bin/bash" "$image_name" -c "
            set -e
            cd /src && 
            # 환경 변수 설정
            export HOME=/root
            export RPMBUILD_DIR=\$HOME/rpmbuild
            
            # RPM 빌드 디렉토리 구조 생성
            mkdir -p \$RPMBUILD_DIR/{BUILD,RPMS,SOURCES,SPECS,SRPMS} &&
            
            # tarball 생성
            VERSION=\$(grep '^Version:' nimf.spec | awk '{print \$2}') &&
            echo \"Creating tarball for version: \${VERSION}\" &&
            
            # Create a clean copy for tarball
            cd /tmp &&
            cp -r /src nimf-\${VERSION} &&
            
            # Remove unwanted files
            find nimf-\${VERSION} -name '.git*' -exec rm -rf {} + 2>/dev/null || true &&
            rm -rf nimf-\${VERSION}/dist 2>/dev/null || true &&
            rm -f nimf-\${VERSION}/*.tar.gz 2>/dev/null || true &&
            
            # Create tarball
            tar czf \$RPMBUILD_DIR/SOURCES/nimf-\${VERSION}.tar.gz nimf-\${VERSION} &&
            
            # Return to source directory and copy spec file
            cd /src &&
            cp nimf.spec \$RPMBUILD_DIR/SPECS/ &&
            
            echo \"Starting RPM build...\" &&
            # RPM 패키지 빌드
            rpmbuild -ba \$RPMBUILD_DIR/SPECS/nimf.spec --define \"_topdir \$RPMBUILD_DIR\" || true &&
            
            # 빌드 결과 확인
            echo \"Checking build results:\" &&
            find \$RPMBUILD_DIR -name '*.rpm' -type f &&
            
            # 생성된 패키지 복사
            mkdir -p /packages/dist/$dockerfile_name &&
            find \$RPMBUILD_DIR -name '*.rpm' -type f -exec cp {} /packages/dist/$dockerfile_name/ \; &&
            
            echo \"RPM build completed\" &&
            ls -la /packages/dist/$dockerfile_name/ 2>/dev/null || echo 'No RPM files found in output directory'
        "; then
            echo -e "${RED}Warning: RPM 패키지 생성 중 오류 발생 - $dockerfile_name${NC}"
        else
            echo -e "${GREEN}RPM 패키지가 $PROJECT_ROOT/dist/$dockerfile_name 디렉토리에 저장되었습니다.${NC}"
        fi
    fi
    
    # Arch 패키지 생성
    if [[ "$dockerfile_name" == *"arch"* ]]; then
        echo -e "${YELLOW}Arch 패키지 생성 중...${NC}"
        if ! docker run --rm -v "$PROJECT_ROOT:/packages" --entrypoint="/bin/bash" "$image_name" -c "
            cd /home/builduser/src && 
            # builduser로 패키지 빌드 (makepkg는 root로 실행할 수 없음)
            su - builduser -c 'cd /home/builduser/src && makepkg -sf --noconfirm' &&
            # 생성된 패키지 복사
            mkdir -p /packages/dist/$dockerfile_name &&
            cp /home/builduser/src/*.pkg.tar.* /packages/dist/$dockerfile_name/ 2>/dev/null &&
            echo -e '${GREEN}Arch 패키지 생성 완료:${NC}' &&
            ls -la /packages/dist/$dockerfile_name/*.pkg.tar.* 2>/dev/null || echo 'No Arch packages created'
        "; then
            echo -e "${RED}Warning: Arch 패키지 생성 중 오류 발생 - $dockerfile_name${NC}"
        else
            echo -e "${GREEN}Arch 패키지가 $PROJECT_ROOT/dist/$dockerfile_name 디렉토리에 저장되었습니다.${NC}"
        fi
    fi
    
    # 대화형 셸 실행
    if [ "$interactive" = true ]; then
        echo -e "${YELLOW}대화형 셸을 실행합니다. 'exit'로 종료하세요.${NC}"
        docker run --rm -it "$image_name"
    fi
    
    # 생성된 패키지 파일 목록 표시
    if [ "$build_only" != true ]; then
        echo -e "\n${BLUE}=== 생성된 패키지 파일 ===${NC}"
        find "$PROJECT_ROOT" -name "*.deb" -o -name "*.rpm" -o -name "*.pkg.tar.*" 2>/dev/null | sort
    fi
    
    echo -e "${GREEN}테스트 완료 - $dockerfile_name${NC}"
}

# 파라미터 파싱
DOCKERFILE=""
BUILD_ONLY=false
PACKAGE_ONLY=false
INTERACTIVE=false
CLEAN=false
TEST_ALL=false

for arg in "$@"; do
    case $arg in
        --build-only)
            BUILD_ONLY=true
            ;;
        --package-only)
            PACKAGE_ONLY=true
            ;;
        --interactive)
            INTERACTIVE=true
            ;;
        --clean)
            CLEAN=true
            ;;
        --all)
            TEST_ALL=true
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        -*)
            echo -e "${RED}Error: 알 수 없는 옵션: $arg${NC}"
            show_usage
            exit 1
            ;;
        *)
            if [ -z "$DOCKERFILE" ]; then
                DOCKERFILE=$arg
            else
                echo -e "${RED}Error: 여러 개의 Dockerfile이 지정되었습니다.${NC}"
                show_usage
                exit 1
            fi
            ;;
    esac
done

# Docker 설치 확인
if ! command -v docker &> /dev/null; then
    echo -e "${RED}Error: Docker가 설치되지 않았습니다.${NC}"
    exit 1
fi

# Docker 실행 권한 확인
if ! docker ps &> /dev/null; then
    echo -e "${RED}Error: Docker를 실행할 권한이 없습니다. sudo를 사용하거나 docker 그룹에 사용자를 추가하세요.${NC}"
    exit 1
fi

# 모든 환경 테스트
if [ "$TEST_ALL" = true ]; then
    echo -e "${BLUE}모든 환경에서 테스트를 시작합니다...${NC}"
    SUCCESS_COUNT=0
    TOTAL_COUNT=${#AVAILABLE_DOCKERFILES[@]}
    
    for dockerfile in "${AVAILABLE_DOCKERFILES[@]}"; do
        dockerfile_name=${dockerfile%.Dockerfile}
        if test_dockerfile "$dockerfile_name" "$BUILD_ONLY" "$PACKAGE_ONLY" false "$CLEAN"; then
            ((SUCCESS_COUNT++))
        fi
    done
    
    echo -e "\n${BLUE}=== 전체 테스트 결과 ===${NC}"
    echo -e "${GREEN}성공: $SUCCESS_COUNT/${TOTAL_COUNT}${NC}"
    
    if [ $SUCCESS_COUNT -eq $TOTAL_COUNT ]; then
        echo -e "${GREEN}모든 환경에서 테스트가 성공했습니다!${NC}"
        exit 0
    else
        echo -e "${RED}일부 환경에서 테스트가 실패했습니다.${NC}"
        exit 1
    fi
fi

# 단일 환경 테스트
if [ -z "$DOCKERFILE" ]; then
    echo -e "${RED}Error: Dockerfile을 지정해야 합니다.${NC}"
    show_usage
    exit 1
fi

# Dockerfile 이름 검증
DOCKERFILE_FOUND=false
for dockerfile in "${AVAILABLE_DOCKERFILES[@]}"; do
    if [ "${dockerfile%.Dockerfile}" = "$DOCKERFILE" ]; then
        DOCKERFILE_FOUND=true
        break
    fi
done

if [ "$DOCKERFILE_FOUND" = false ]; then
    echo -e "${RED}Error: 지원되지 않는 Dockerfile: $DOCKERFILE${NC}"
    show_usage
    exit 1
fi

# 테스트 실행
test_dockerfile "$DOCKERFILE" "$BUILD_ONLY" "$PACKAGE_ONLY" "$INTERACTIVE" "$CLEAN"
