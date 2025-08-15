# GitHub Actions 로컬 테스트 방법

## Act 도구 설치 및 사용

```bash
# Act 설치 (Ubuntu/Debian)
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# 또는 GitHub에서 직접 다운로드
wget https://github.com/nektos/act/releases/latest/download/act_Linux_x86_64.tar.gz
tar xzf act_Linux_x86_64.tar.gz
sudo mv act /usr/local/bin/

# workflow 로컬 테스트 실행
act -j build-packages  # 특정 job만 테스트
act                     # 전체 workflow 테스트
```

## GitHub CLI를 사용한 원격 테스트

```bash
# GitHub CLI 설치 후
gh auth login

# workflow 수동 실행
gh workflow run "Test Multi-Platform Release with Docker Build"

# 실행 상태 확인
gh run list
gh run view [run-id]
```