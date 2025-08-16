FROM archlinux:latest AS builder

# 필요한 패키지 설치
RUN pacman -Syu --noconfirm \
    binutils \
    autoconf \
    automake \
    gcc \
    make \
    glib2 \
    glib2-devel \
    gtk3 \
    gtk2 \
    qt5-base \
    qt6-base \
    libappindicator-gtk3 \
    libayatana-appindicator \
    librsvg \
    noto-fonts-cjk \
    anthy \
    librime \
    libxkbcommon \
    wayland \
    wayland-protocols \
    libxklavier \
    m17n-lib \
    m17n-db \
    gtk-doc \
    git \
    base-devel \
    sudo \
    wget \
    intltool \
    libtool \
    pkg-config

# 일반 사용자 생성
RUN useradd -m builduser && echo "builduser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# 소스 코드 복사 및 작업 디렉토리 설정
COPY . /home/builduser/src
RUN chown -R builduser:builduser /home/builduser/src

# 일반 사용자로 전환
USER builduser
WORKDIR /home/builduser/src

# 네트워크 연결 확인
RUN echo "Testing network connectivity..." && \
    ping -c 3 aur.archlinux.org || echo "AUR connectivity test failed, but continuing..."

# AUR 도우미 yay 설치 (개선된 재시도 로직)
RUN echo "Installing yay AUR helper..." && \
    for i in 1 2 3 4 5; do \
        echo "Attempt $i to clone yay..." && \
        rm -rf /home/builduser/yay 2>/dev/null || true && \
        git clone https://aur.archlinux.org/yay.git /home/builduser/yay && \
        echo "Successfully cloned yay on attempt $i" && break || \
        echo "Attempt $i failed, waiting 10 seconds before retry..." && sleep 10; \
    done && \
    if [ ! -d "/home/builduser/yay" ]; then \
        echo "Failed to clone yay after 5 attempts" && exit 1; \
    fi && \
    cd /home/builduser/yay && \
    makepkg -si --noconfirm

# AUR에서 libhangul-git 패키지 설치 (재시도 로직 포함)
RUN echo "Installing libhangul-git from AUR..." && \
    for i in 1 2 3; do \
        echo "Attempt $i to install libhangul-git..." && \
        yay -S --noconfirm libhangul-git && \
        echo "Successfully installed libhangul-git on attempt $i" && break || \
        echo "Attempt $i failed, waiting 15 seconds before retry..." && sleep 15; \
    done

# 패키지 빌드 및 설치
# RUN makepkg -si --noconfirm

# 루트 사용자로 전환
USER root
RUN rm -rf /home/builduser/yay

# 프로젝트 빌드 (패키지 빌드용)
RUN cd /home/builduser/src && \
    echo "Cleaning up previous build artifacts..." && \
    make clean || echo "No Makefile found, skipping make clean" && \
    find . -name "*.moc" -delete 2>/dev/null || true && \
    find . -name "moc_*.cpp" -delete 2>/dev/null || true && \
    echo "Checking for intltoolize..." && \
    which intltoolize || echo "intltoolize not found in PATH" && \
    echo "Running autogen.sh..." && \
    ./autogen.sh --prefix=/usr --enable-gtk-doc && \
    echo "Building project..." && \
    make -j $(nproc) 

ENTRYPOINT ["bash"]
