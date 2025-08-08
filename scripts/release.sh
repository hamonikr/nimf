#!/bin/bash

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 스크립트의 실제 위치 확인
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# 프로젝트 루트 디렉토리 (스크립트의 상위 디렉토리)
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# 프로젝트 루트 디렉토리로 이동
cd "$PROJECT_ROOT"

echo -e "${YELLOW}프로젝트 디렉토리: ${NC}$PROJECT_ROOT"

# 현재 브랜치 확인
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$CURRENT_BRANCH" != "dev" ]; then
    echo -e "${RED}Error: dev 브랜치에서만 실행할 수 있습니다.${NC}"
    echo -e "${YELLOW}현재 브랜치: ${NC}$CURRENT_BRANCH"
    exit 1
fi

# 커밋되지 않은 변경사항 확인
if [ -n "$(git status --porcelain)" ]; then
    echo -e "${RED}Error: 커밋되지 않은 변경사항이 있습니다. 먼저 모든 변경사항을 커밋해주세요.${NC}"
    exit 1
fi

# master와 dev 브랜치의 차이 확인
git fetch origin master dev
MASTER_HEAD=$(git rev-parse origin/master)
DEV_HEAD=$(git rev-parse origin/dev)

if [ "$MASTER_HEAD" == "$DEV_HEAD" ]; then
    echo -e "${RED}Error: master 브랜치와 dev 브랜치의 내용이 동일합니다.${NC}"
    echo -e "${YELLOW}릴리즈할 새로운 변경사항이 없습니다.${NC}"
    exit 1
fi

# 현재 버전 가져오기 (configure.ac에서 추출)
CURRENT_VERSION_FROM_CONFIG=$(grep "AC_INIT" configure.ac | sed -n 's/AC_INIT(nimf, \([^)]*\))/\1/p')
CURRENT_VERSION="v$CURRENT_VERSION_FROM_CONFIG"

# Git 태그에서도 확인하여 더 높은 버전이 있으면 사용
GIT_VERSION=$(git tag --sort=-v:refname | grep '^v' | head -n1 || echo "v0.0.0")
if [ "$GIT_VERSION" != "v0.0.0" ]; then
    # 버전 비교 (간단한 비교)
    if [[ "$GIT_VERSION" > "$CURRENT_VERSION" ]]; then
        CURRENT_VERSION="$GIT_VERSION"
    fi
fi

# 버전 증가 함수
increment_version() {
    local version=$1
    local increment_type=$2
    
    # v 접두사 제거
    version=${version#v}
    
    # 버전을 . 으로 분리
    IFS='.' read -ra VERSION_PARTS <<< "$version"
    
    major=${VERSION_PARTS[0]:-0}
    minor=${VERSION_PARTS[1]:-0}
    patch=${VERSION_PARTS[2]:-0}
    
    case $increment_type in
        major)
            major=$((major + 1))
            minor=0
            patch=0
            ;;
        minor)
            minor=$((minor + 1))
            patch=0
            ;;
        patch|*)
            patch=$((patch + 1))
            ;;
    esac
    
    echo "v$major.$minor.$patch"
}

# 파라미터 파싱
AUTO_YES=false
VERSION_TYPE="patch"

for arg in "$@"; do
    case $arg in
        --yes|-yes|-y|-Y)
            AUTO_YES=true
            # -y 옵션만 사용하면 기본적으로 patch 버전으로 설정
            if [ $# -eq 1 ]; then
                VERSION_TYPE="patch"
            fi
            ;;
        major|minor|patch)
            VERSION_TYPE=$arg
            ;;
        *)
            echo -e "${RED}Error: 알 수 없는 파라미터: $arg${NC}"
            echo -e "${YELLOW}사용법: $0 [major|minor|patch] [--yes|-yes|-y|-Y]${NC}"
            echo -e "${YELLOW}       $0 -y  (patch 버전 자동 증가)${NC}"
            echo -e "${YELLOW}이 스크립트는 configure.ac, PKGBUILD, nimf.spec 버전을 업데이트합니다.${NC}"
            exit 1
            ;;
    esac
done

# 버전 타입 확인
if [[ ! "$VERSION_TYPE" =~ ^(major|minor|patch)$ ]]; then
    echo -e "${RED}Error: 버전 타입은 major, minor, patch 중 하나여야 합니다.${NC}"
    exit 1
fi

# 새 버전 생성
NEW_VERSION=$(increment_version $CURRENT_VERSION $VERSION_TYPE)

# configure.ac 버전 업데이트
echo -e "\n${YELLOW}configure.ac 버전 업데이트 중...${NC}"
echo -e "${YELLOW}업데이트 전 configure.ac 버전:${NC}"
grep "AC_INIT" configure.ac

# OS 확인 후 적절한 sed 명령어 사용
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    sed -i '' "s/AC_INIT(nimf, .*))/AC_INIT(nimf, ${NEW_VERSION#v})/" configure.ac
else
    # Linux 및 기타
    sed -i "s/AC_INIT(nimf, .*))/AC_INIT(nimf, ${NEW_VERSION#v})/" configure.ac
fi

echo -e "${YELLOW}업데이트 후 configure.ac 버전:${NC}"
grep "AC_INIT" configure.ac

# PKGBUILD 버전 업데이트
if [ -f "PKGBUILD" ]; then
    echo -e "\n${YELLOW}PKGBUILD 버전 업데이트 중...${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^pkgver=.*/pkgver=${NEW_VERSION#v}/" PKGBUILD
    else
        sed -i "s/^pkgver=.*/pkgver=${NEW_VERSION#v}/" PKGBUILD
    fi
    echo -e "${GREEN}PKGBUILD 버전 업데이트 완료${NC}"
fi

# nimf.spec 버전 업데이트
if [ -f "nimf.spec" ]; then
    echo -e "\n${YELLOW}nimf.spec 버전 업데이트 중...${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^%define version .*$/%define version ${NEW_VERSION#v}/" nimf.spec
    else
        sed -i "s/^%define version .*$/%define version ${NEW_VERSION#v}/" nimf.spec
    fi
    echo -e "${GREEN}nimf.spec 버전 업데이트 완료${NC}"
fi

# debian/changelog 업데이트
if [ -f "debian/changelog" ]; then
    echo -e "\n${YELLOW}debian/changelog 버전 업데이트 중...${NC}"
    
    # 현재 배포판 이름 가져오기 (기본값: unstable)
    DISTRIBUTION=$(head -n1 debian/changelog | awk '{print $3}' | tr -d ';' || echo "unstable")
    
    # 변경사항 수집 (master와 dev 브랜치 간 차이)
    CHANGELOG_ENTRIES=""
    while IFS= read -r line; do
        # 커밋 메시지에서 feat:, fix:, chore: 등을 제거하고 정리
        CLEAN_MSG=$(echo "$line" | sed -E 's/^[a-f0-9]+ //' | sed -E 's/^(feat|fix|chore|docs|style|refactor|test|build):\s*//')
        if [ -n "$CLEAN_MSG" ]; then
            CHANGELOG_ENTRIES="${CHANGELOG_ENTRIES}  * ${CLEAN_MSG}\n"
        fi
    done < <(git --no-pager log --oneline origin/master..origin/dev --pretty=format:"%h %s")
    
    # dch 명령어가 있는지 확인
    if command -v dch &> /dev/null; then
        # dch를 사용하여 새 엔트리 추가
        DEBEMAIL="${DEBEMAIL:-root@hamonikr.org}" \
        DEBFULLNAME="${DEBFULLNAME:-HamoniKR}" \
        dch --newversion "${NEW_VERSION#v}" --distribution "$DISTRIBUTION" "Release ${NEW_VERSION}"
        
        # 변경사항 추가
        if [ -n "$CHANGELOG_ENTRIES" ]; then
            echo -e "$CHANGELOG_ENTRIES" | while IFS= read -r entry; do
                if [ -n "$entry" ]; then
                    dch --append "$entry"
                fi
            done
        fi
    else
        # dch가 없으면 수동으로 업데이트
        echo -e "${YELLOW}dch 명령어가 없습니다. 수동으로 debian/changelog를 업데이트합니다.${NC}"
        
        # 현재 날짜를 debian changelog 형식으로
        CHANGELOG_DATE=$(date -R)
        
        # 새 changelog 엔트리 생성
        NEW_ENTRY="nimf (${NEW_VERSION#v}) $DISTRIBUTION; urgency=medium\n\n"
        if [ -n "$CHANGELOG_ENTRIES" ]; then
            NEW_ENTRY="${NEW_ENTRY}${CHANGELOG_ENTRIES}"
        else
            NEW_ENTRY="${NEW_ENTRY}  * Release ${NEW_VERSION}\n"
        fi
        NEW_ENTRY="${NEW_ENTRY}\n -- ${DEBFULLNAME:-HamoniKR} <${DEBEMAIL:-root@hamonikr.org}>  $CHANGELOG_DATE\n\n"
        
        # 기존 changelog 백업
        cp debian/changelog debian/changelog.bak
        
        # 새 엔트리를 파일 맨 위에 추가
        echo -e "$NEW_ENTRY" > debian/changelog.tmp
        cat debian/changelog.bak >> debian/changelog.tmp
        mv debian/changelog.tmp debian/changelog
    fi
    
    echo -e "${YELLOW}debian/changelog 업데이트 완료${NC}"
    head -n6 debian/changelog
fi

# 변경사항 요약 표시
echo -e "\n${YELLOW}변경사항 요약:${NC}"
git --no-pager log --oneline origin/master..origin/dev

echo -e "\n${YELLOW}현재 버전: ${NC}$CURRENT_VERSION"
echo -e "${YELLOW}새로운 버전: ${NC}$NEW_VERSION"
echo -e "\n${GREEN}릴리즈 프로세스를 시작합니다...${NC}"

# 사용자 확인
if [ "$AUTO_YES" = true ]; then
    echo -e "\n${YELLOW}--yes 옵션이 설정되어 릴리즈를 자동으로 진행합니다.${NC}"
else
    read -p "계속 진행하시겠습니까? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${RED}릴리즈가 취소되었습니다.${NC}"
        exit 1
    fi
fi

# 버전 변경사항 커밋
echo -e "\n${YELLOW}버전 변경사항 커밋${NC}"
git add configure.ac
if [ -f "PKGBUILD" ]; then
    git add PKGBUILD
fi
if [ -f "nimf.spec" ]; then
    git add nimf.spec
fi
if [ -f "debian/changelog" ]; then
    git add debian/changelog
fi
git commit -m "chore: bump version to ${NEW_VERSION}"

# 릴리즈 프로세스 실행
echo -e "\n${YELLOW}1. master 브랜치로 전환${NC}"
git checkout master

echo -e "\n${YELLOW}2. master 브랜치 업데이트${NC}"
git pull origin master

echo -e "\n${YELLOW}3. dev 브랜치 머지${NC}"
git merge dev

echo -e "\n${YELLOW}4. 새로운 태그 생성${NC}"
git tag -a $NEW_VERSION -m "Release $NEW_VERSION"

echo -e "\n${YELLOW}5. 변경사항 푸시${NC}"
git push origin master

echo -e "\n${YELLOW}6. 태그 푸시${NC}"
git push origin $NEW_VERSION

echo -e "\n${YELLOW}7. dev 브랜치로 복귀${NC}"
git checkout dev

echo -e "\n${YELLOW}8. dev 브랜치에 master의 변경사항 동기화${NC}"
git merge master
git push origin dev

echo -e "\n${GREEN}릴리즈 완료! 새 버전 ${NEW_VERSION}이 성공적으로 배포되었습니다.${NC}"
echo -e "${YELLOW}GitHub에서 릴리즈 노트를 작성하는 것을 잊지 마세요!${NC}"
