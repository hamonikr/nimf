#!/bin/bash

# GitHub 원격 저장소 URL 가져오기
REMOTE_URL=$(git config --get remote.origin.url)

# 사용자 이름과 저장소 이름 추출
if [[ $REMOTE_URL =~ ^git@github.com:(.*)/(.*).git$ ]]; then
  GITHUB_USER="${BASH_REMATCH[1]}"
  GITHUB_REPO="${BASH_REMATCH[2]}"
elif [[ $REMOTE_URL =~ ^https://github.com/(.*)/(.*).git$ ]]; then
  GITHUB_USER="${BASH_REMATCH[1]}"
  GITHUB_REPO="${BASH_REMATCH[2]}"
else
  echo "지원되지 않는 원격 저장소 URL 형식입니다."
  exit 1
fi

# 파라미터 확인
if [ "$1" == "release" ]; then
  # 태그 목록 가져오기
  tags=$(git tag --sort=-creatordate)

  previous_tag=""
  latest_tag=""

  # 최신 태그와 이전 태그 가져오기
  for tag in $tags; do
    if [ -n "$latest_tag" ]; then
      previous_tag=$tag
      break
    fi
    latest_tag=$tag
  done

  # 이전 태그와 최신 태그 사이의 로그 추출
  if [ -n "$previous_tag" ]; then
    echo "## [$latest_tag](https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/tag/$latest_tag)"
    echo ""
    git log --pretty=format:"- %ad %an: %s ([%h](https://github.com/$GITHUB_USER/$GITHUB_REPO/commit/%H))" --date=short $previous_tag..$latest_tag --no-merges
    echo ""
  else
    # 이전 태그가 없을 경우 전체 로그 추출
    echo "## [$latest_tag](https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/tag/$latest_tag)"
    echo ""
    git log --pretty=format:"- %ad %an: %s ([%h](https://github.com/$GITHUB_USER/$GITHUB_REPO/commit/%H))" --date=short $latest_tag --no-merges
    echo ""
  fi
else
  # 전체 로그 추출
  echo "# Changelog" > CHANGELOG.md
  echo "" >> CHANGELOG.md

  tags=$(git tag --sort=-creatordate)

  previous_tag=""

  for tag in $tags; do
    if [ -n "$previous_tag" ]; then
      echo "## [$previous_tag](https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/tag/$previous_tag)" >> CHANGELOG.md
      echo "" >> CHANGELOG.md
      git log --pretty=format:"- %ad %an: %s ([%h](https://github.com/$GITHUB_USER/$GITHUB_REPO/commit/%H))" --date=short $tag..$previous_tag --no-merges >> CHANGELOG.md
      echo "" >> CHANGELOG.md
    fi
    previous_tag=$tag
  done

  # 최신 태그의 로그 추가
  if [ -n "$previous_tag" ]; then
    echo "## [$previous_tag](https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/tag/$previous_tag)" >> CHANGELOG.md
    echo "" >> CHANGELOG.md
    git log --pretty=format:"- %ad %an: %s ([%h](https://github.com/$GITHUB_USER/$GITHUB_REPO/commit/%H))" --date=short $previous_tag --no-merges >> CHANGELOG.md
    echo "" >> CHANGELOG.md

    echo "Changelog가 생성되었습니다."
  fi
fi


